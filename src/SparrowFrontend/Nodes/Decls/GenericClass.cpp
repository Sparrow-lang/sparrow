#include <StdInc.h>
#include "GenericClass.h"
#include "Instantiation.h"
#include "SprClass.h"
#include <Helpers/StdDef.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/DeclsHelpers.h>

#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Decl.h>

#include <Nest/Intermediate/Modifier.h>
#include <Nest/Frontend/SourceCode.h>

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;


namespace
{
    /// Get the bound arguments corresponding to the arguments passed to the generic
    /// We return here the CT values of these arguments; we use their value to check for duplicate instantiations
    NodeVector getBoundValues(const NodeVector& args)
    {
        NodeVector boundValues;
        boundValues.reserve(args.size());

        for ( size_t i=0; i<args.size(); ++i )
        {
            Node* arg = args[i];

            // Evaluate the node and add the resulting CtValue as a bound argument
            arg->computeType();
            if ( !Feather::isCt(arg) )
                REP_INTERNAL(arg->location(), "Argument to a class generic must be CT (type: %1%)") % arg->type();
            Node* n = theCompiler().ctEval(arg);
            if ( !n || n->nodeKind() != nkFeatherExpCtValue )
                REP_INTERNAL(arg->location(), "Invalid argument %1% when instantiating generic") % (i+1);
            boundValues.push_back(n);
        }
        return boundValues;
    }

    EvalMode getResultingEvalMode(const Location& loc, EvalMode mainEvalMode, const NodeVector& boundValues)
    {
        bool hasRtOnlyArgs = false;
        bool hasCtOnlyArgs = false;
        for ( Node* boundVal: boundValues )
        {
            // Test the type given to the 'Type' parameters (i.e., we need to know if Vector(t) can be rtct based on the mode of t)
            TypeRef t = tryGetTypeValue(boundVal);
            if ( t )
            {
                if ( t->mode == modeRt )
                    hasRtOnlyArgs = true;
                else if ( t->mode == modeCt )
                    hasCtOnlyArgs = true;
            }
            else if ( !boundVal->type()->canBeUsedAtRt )
            {
                hasCtOnlyArgs = true;
            }
        }
        if ( hasCtOnlyArgs && hasRtOnlyArgs )
            REP_ERROR(loc, "Cannot instantiate generic with both RT-only and CT-only arguments");
        if ( mainEvalMode == modeCt && hasRtOnlyArgs )
            REP_ERROR(loc, "Cannot use RT-only arguments in a CT generic");
        if ( mainEvalMode == modeRt && hasCtOnlyArgs )
            REP_ERROR(loc, "Cannot use CT-only arguments in a RT generic");

        if ( hasCtOnlyArgs )
            return modeCt;
        if ( hasRtOnlyArgs )
            return modeRt;
        return mainEvalMode;
    }

    Node* createInstantiatedClass(CompilationContext* context, SprClass* orig, const string& description)
    {
        const Location& loc = orig->location();

        NodeList* baseClasses = orig->baseClasses();
        NodeList* children = orig->classChildren();
        baseClasses = baseClasses ? baseClasses->clone() : nullptr;
        children = children ? children->clone() : nullptr;
        Node* newClass = mkSprClass(loc, getName(orig), nullptr, baseClasses, nullptr, children);

        copyModifiersSetMode(orig, newClass, context->evalMode());

        //setShouldAddToSymTab(newClass, false);    // TODO (generics): Uncomment this line
        newClass->setContext(context);

//        REP_INFO(loc, "Instantiated %1%") % description;
        return newClass;
    }

    string getDescription(SprClass* cls, Instantiation* inst)
    {
        ostringstream oss;
        oss << getName(cls) << "[";
        const auto& boundValues = inst->boundValues();
        for ( size_t i=0; i<boundValues.size(); ++i )
        {
            if ( i>0 )
                oss << ", ";
            TypeRef t = evalTypeIfPossible(boundValues[i]);
            if ( t )
                oss << t;
            else
                oss << boundValues[i];
        }
        oss << "]";
        return oss.str();
    }
}


GenericClass::GenericClass(SprClass* originalClass, NodeList* parameters, Node* ifClause)
    : Generic(classNodeKind(), originalClass, parameters->children(), ifClause, publicAccess)
{
    setEvalMode(this, effectiveEvalMode(originalClass));

    // Semantic check the arguments
    for ( Node* param: parameters->children() )
    {
        param->semanticCheck();
        if ( isConceptType(param->type()) )
            REP_ERROR(param->location(), "Cannot use auto or concept parameters for class generics");
    }
}

size_t GenericClass::paramsCount() const
{
    InstantiationsSet* instantiationsSet = children_[0]->as<InstantiationsSet>();
    return instantiationsSet->parameters().size();
}

Node* GenericClass::param(size_t idx) const
{
    InstantiationsSet* instantiationsSet = children_[0]->as<InstantiationsSet>();
    return instantiationsSet->parameters()[idx];
}

Instantiation* GenericClass::canInstantiate(const NodeVector& args)
{
    NodeVector boundValues = getBoundValues(args);
    Node* originalClass = referredNodes_[0];
    EvalMode resultingEvalMode = getResultingEvalMode(originalClass->location(), effectiveEvalMode(originalClass), boundValues);
    InstantiationsSet* instantiationsSet = children_[0]->as<InstantiationsSet>();
    return instantiationsSet->canInstantiate(boundValues, resultingEvalMode);
}

Node* GenericClass::instantiateGeneric(const Location& loc, CompilationContext* context, const NodeVector& /*args*/, Instantiation* inst)
{
    ASSERT(inst);

    // If not already created, create the actual instantiation declaration
    Node* instantiatedDecl = inst->instantiatedDecl();
    NodeList* expandedInstantiation = inst->expandedInstantiation();
    if ( !instantiatedDecl )
    {
        SprClass* originalClass = referredNodes_[0]->as<SprClass>();
        string description = getDescription(originalClass, inst);

        // Create the actual instantiation declaration
        CompilationContext* ctx = expandedInstantiation->childrenContext();
        instantiatedDecl = createInstantiatedClass(ctx, originalClass, description);
        if ( !instantiatedDecl )
            REP_INTERNAL(loc, "Cannot instantiate generic");
        instantiatedDecl->setProperty(propDescription, move(description));
        instantiatedDecl->computeType();
        theCompiler().queueSemanticCheck(instantiatedDecl);
        inst->setInstantiatedDecl(instantiatedDecl);

        // Add the instantiated class as an additional node to the callee source code
        ASSERT(context->sourceCode());
        context->sourceCode()->addAdditionalNode(expandedInstantiation);
    }

    // Now actually create the call object: a Type CT value
    Class* cls = instantiatedDecl->explanation()->as<Class>();
    ASSERT(cls);
    return createTypeNode(context_, loc, Feather::getDataType(cls));
}
