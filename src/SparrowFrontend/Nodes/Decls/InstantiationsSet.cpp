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
    NodeVector getBoundVariables(const Location& loc, const NodeVector& boundValues, const NodeVector& params, bool insideClass)
    {
        // Create a variable for each bound parameter - put everything in a node list
        NodeVector nodes;
        NodeVector nonBoundParams;
        size_t idx = 0;
        for ( Node* p: params )
        {
            Node* boundValue = boundValues[idx++];
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
        return nodes;
    }
}


InstantiationsSet::InstantiationsSet(Node* parentNode, NodeVector params, Node* ifClause)
    : Node(parentNode->location(), { ifClause, Feather::mkNodeList(parentNode->location(), {}) }, { parentNode })
{
    referredNodes_.push_back(mkNodeList(location_, move(params)));
}

Instantiation* InstantiationsSet::canInstantiate(const NodeVector& values, EvalMode evalMode)
{
    // Try to find an existing instantiation
    Instantiation* inst = searchInstantiation(values);
    if ( inst )
    {
        // We already checked whether we can instantiate this
        return inst->isValid() ? inst : nullptr;
    }

    // If no instantiation is found, create a new instantiation
    inst = createNewInstantiation(values, evalMode);

    // If we have an if clause, check if this CT evaluates to true
    if ( ifClause() )
    {
        // Always use a clone of the original node
        Node* cond = ifClause()->clone();
        cond->setContext(inst->expandedInstantiation()->childrenContext());

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

    inst->setValid();
    return inst;
}

const NodeVector& InstantiationsSet::parameters() const
{
    return referredNodes_[1]->children();
}

Instantiation* InstantiationsSet::searchInstantiation(const NodeVector& values)
{
    for ( Instantiation* inst: instantiations() )
    {
        const auto& boundValues = inst->boundValues();
        if ( boundValues.size() != values.size() )
            continue;

        bool argsMatch = true;
        for ( size_t i=0; i<values.size(); ++i )
        {
            if ( !boundValues[i] )
                continue;
            if ( !values[i] || !ctValsEqual(values[i], boundValues[i]) )
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
    CompilationContext* context = parentNode()->context()->createChildContext(nullptr);
    context->setEvalMode(evalMode);
    bool insideClass = nullptr != getParentClass(context);

    // Create the instantiation
    auto boundVars = getBoundVariables(location_, values, parameters(), insideClass);
    Instantiation* inst = new Instantiation(location_, values, move(boundVars));
    instantiations().push_back(inst);

    // Compile the newly created instantiation
    inst->expandedInstantiation()->setContext(context);
    inst->expandedInstantiation()->semanticCheck();

    return inst;
}

Node* InstantiationsSet::parentNode() const
{
    return referredNodes_[0];
}

Node*  InstantiationsSet::ifClause() const
{
    return children_[0];
}

vector<Instantiation*>& InstantiationsSet::instantiations()
{
    return reinterpret_cast<vector<Instantiation*>&>(children_[1]->children());
}
