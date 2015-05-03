#include <StdInc.h>
#include "InstantiationsSet.h"
#include "Instantiation.h"
#include "SprVariable.h"
#include <Helpers/Ct.h>
#include <Helpers/SprTypeTraits.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Context.h>
#include <Feather/Util/Decl.h>
#include <Feather/Util/Ct.h>

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace
{
    /// In a generic we need to be able to access the bound variables; moreover the if clause might reference them.
    /// For this, we create a set of variables corresponding to the instantiation bound values. These variables are
    /// created for each instantiated and put in the node-list of the instantiation (the expanded instantiation node).
    /// Note that for auto-parameters we will create RT variables; the only thing we can do with them is to use their type
    /// In the expanded instantiation we need to add the actual instantiated declaration - in other place, not here
    void addBoundVariables(const Location& loc, Instantiation& inst, const NodeVector& params, bool insideClass)
    {
        ASSERT(!inst.expandedInstantiation_);

        // Create a variable for each bound parameter - put everything in a node list
        NodeVector nodes;
        NodeVector nonBoundParams;
        size_t idx = 0;
        for ( Node* p: params )
        {
            Node* boundValue = inst.boundValues_[idx++];
            if ( !p )
                continue;
            ASSERT(boundValue);

            if ( isConceptType(p->type()) )
            {
                Type* t = getType(boundValue);

                Node* var = mkSprVariable(p->location(), getName(p), t, nullptr);
                if ( insideClass )
                    var->setProperty(propIsStatic, 1);
                nodes.push_back(var);
            }
            else
            {
                Node* var = (Node*) mkSprVariable(p->location(), getName(p), boundValue->type(), boundValue);
                if ( insideClass )
                    var->setProperty(propIsStatic, 1);
                setEvalMode(var, modeCt);
                nodes.push_back(var);
            }
        }
        nodes.push_back(mkNop(loc));    // Make sure the resulting type is Void
        inst.expandedInstantiation_ = (NodeList*) mkNodeList(loc, move(nodes));
    }
}


InstantiationsSet::InstantiationsSet(Node* parentNode, NodeVector params, Node* ifClause)
    : parentNode_(parentNode)
    , params_(move(params))
    , ifClause_(ifClause)
{

}

InstantiationsSet::~InstantiationsSet()
{
    for ( Instantiation* inst: instantiations_ )
    {
        delete inst;
    }
}

Instantiation* InstantiationsSet::canInstantiate(const NodeVector& values, EvalMode evalMode)
{
    // Try to find an existing instantiation
    Instantiation* inst = searchInstantiation(values);
    if ( inst )
    {
        // We already checked whether we can instantiate this
        return inst->isValid_ ? inst : nullptr;
    }

    // If no instantiation is found, create a new instantiation
    inst = createNewInstantiation(values, evalMode);

    // If we have an if clause, check if this CT evaluates to true
    if ( ifClause_ )
    {
        // Always use a clone of the original node
        Node* cond = ifClause_->clone();
        cond->setContext(inst->expandedInstantiation_->childrenContext());

        // If the condition does not compile, we cannot instantiate
        bool isValid = false;
        Nest::Common::DiagnosticSeverity level = Nest::theCompiler().diagnosticReporter().severityLevel();
        try
        {
            Nest::theCompiler().diagnosticReporter().setSeverityLevel(Nest::Common::diagInternalError);
            cond->semanticCheck();
            isValid = !cond->hasError()
                && Feather::isCt(cond)          // We must have a value at CT
                && Feather::isTestable(cond);   // The value must be boolean
        }
        catch (...)
        {
        }
        Nest::theCompiler().diagnosticReporter().setSeverityLevel(level);
        if ( !isValid )
            return nullptr;

        // Evaluate the if clause condition and check the result
        if ( !getBoolCtValue(theCompiler().ctEval(cond)) )
            return nullptr;
    }

    inst->isValid_ = true;
    return inst;
}

Instantiation* InstantiationsSet::searchInstantiation(const NodeVector& values)
{
    for ( Instantiation* inst: instantiations_ )
    {
        if ( inst->boundValues_.size() != values.size() )
            continue;

        bool argsMatch = true;
        for ( size_t i=0; i<values.size(); ++i )
        {
            if ( !inst->boundValues_[i] )
                continue;
            if ( !values[i] || !ctValsEqual(values[i], inst->boundValues_[i]) )
            {
                argsMatch = false;
                break;
            }
        }
        if ( argsMatch )
            return inst;
    }
    return nullptr;
}

Instantiation* InstantiationsSet::createNewInstantiation(const NodeVector& values, EvalMode evalMode)
{
    // Create a new context, but at the same level as the context of the parent node
    CompilationContext* context = parentNode_->context()->createChildContext(nullptr);
    context->setEvalMode(evalMode);
    bool insideClass = nullptr != getParentClass(context);

    // Create the instantiation
    Instantiation* inst = new Instantiation;
    inst->boundValues_ = values;
    addBoundVariables(parentNode_->location(), *inst, params_, insideClass);

    instantiations_.push_back(inst);

    // Compile the newly created instantiation
    inst->expandedInstantiation_->setContext(context);
    inst->expandedInstantiation_->semanticCheck();

    return inst;
}
