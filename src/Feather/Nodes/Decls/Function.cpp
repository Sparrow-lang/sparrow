#include <StdInc.h>
#include "Function.h"
#include "Nodes/Properties.h"

#include <Feather/FeatherTypes.h>
#include <Util/TypeTraits.h>
#include <Util/Decl.h>

#include <Nest/Common/Diagnostic.h>
#include <Nest/Intermediate/CompilationContext.h>

using namespace Feather;


Function::Function(const Location& loc, string name, DynNode* resultType, DynNode* body, DynNodeVector parameters, CallConvention callConv)
    : DynNode(classNodeKind(), loc, move(parameters))
{
    // Make sure all the nodes given as parameters have the right kind
    for ( DynNode* param: parameters )
    {
        if ( param->explanation()->nodeKind() != nkFeatherDeclVar )
            REP_INTERNAL(param->location(), "DynNode %1% must be a parameter") % param;
    }

    // The result type and body is at the beginning of the parameters
    data_.children.insert(data_.children.begin(), body->node());
    data_.children.insert(data_.children.begin(), resultType->node());

    setName(node(), move(name));
    setProperty("callConvention", (int) callConv);
}

void Function::addParameter(DynNode* parameter, bool first)
{
    if ( parameter->explanation()->nodeKind() != nkFeatherDeclVar )
        REP_INTERNAL(parameter->location(), "DynNode %1% must be a parameter") % parameter;

    ASSERT(data_.children.size() >= 2);
    if ( first )
        data_.children.insert(data_.children.begin()+2, parameter->node());
    else
        data_.children.push_back(parameter->node());
}

void Function::setResultType(DynNode* resultType)
{
    ASSERT(data_.children.size() >= 2);
    data_.children[0] = resultType->node();
    resultType->setContext(data_.childrenContext);
}

void Function::setBody(DynNode* body)
{
    ASSERT(data_.children.size() >= 2);
    data_.children[1] = body->node();
}

size_t Function::numParameters() const
{
    return data_.children.size()-2;
}
DynNode* Function::getParameter(size_t idx) const
{
    return (DynNode*) data_.children[idx+2];
}

TypeRef Function::resultType() const
{
    ASSERT(data_.children.size() >= 2);
    return data_.children[0]->type;
}

DynNode* Function::body() const
{
    ASSERT(data_.children.size() >= 2);
    return (DynNode*) data_.children[1];
}

CallConvention Function::callConvention() const
{
    return (CallConvention) getCheckPropertyInt("callConvention");
}


void Function::dump(ostream& os) const
{
    os << endl;
    os << "fun(\"" << getName(node()) << "\"";
    auto it = data_.children.begin()+2;
    auto ite = data_.children.end();
    for ( ; it!=ite; ++it )
    {
        os << ", " << *it;
    }
    os << "): " << resultType() << " {" << endl;
    os << data_.children[1];
    os << "}";
}

string Function::toString() const
{
    ostringstream oss;
    oss << getName(node());
    if ( data_.type )
    {
        oss << '(';
        bool hasResultParam = hasProperty(propResultParam);
        size_t startIdx = hasResultParam ? 3 : 2;
        for ( size_t i=startIdx; i<data_.children.size(); ++i )
        {
            if ( i > startIdx )
                oss << ", ";
            oss << data_.children[i]->type;
        }
        oss << "): " << (hasResultParam ? removeRef(data_.children[2]->type) : resultType());
    }
    return oss.str();
//    return data_.type ? getName(this) + data_.type->toString() : getName(this);
}

void Function::doSetContextForChildren()
{
    // If we don't have a children context, create one
    if ( !data_.childrenContext )
        data_.childrenContext = data_.context->createChildContext(node(), effectiveEvalMode(node()));

    DynNode::doSetContextForChildren();
    
    addToSymTab(node());
}

void Function::doComputeType()
{
    if ( getName(node()).empty() )
        REP_ERROR(data_.location, "No name given to function declaration");

    // We must have a result type
    DynNode* resultType = (DynNode*) data_.children[0];
    resultType->computeType();
    TypeRef resType = resultType->type();
    if ( !resType )
        REP_ERROR(data_.location, "No result type given to function %1%") % getName(node());

    // Compute the type for all the parameters
    auto it = data_.children.begin()+2;
    auto ite = data_.children.end();
    for ( ; it!=ite; ++it )
    {
        Node* param = *it;
        if ( !param )
            REP_ERROR(data_.location, "Invalid param");
        Nest::computeType(param);
    }

    // Set the type for this node
    vector<TypeRef> subTypes;
    subTypes.reserve(data_.children.size()-1);
    subTypes.push_back(resType);
    it = data_.children.begin()+2;
    for ( ; it!=ite; ++it )
    {
        Node* param = *it;
        subTypes.push_back(param->type);
    }
    data_.type = getFunctionType(&subTypes[0], subTypes.size(), effectiveEvalMode(node()));
}

void Function::doSemanticCheck()
{
    // Make sure the type is computed
    computeType();

    // Semantically check all the parameters
    auto it = data_.children.begin()+2;
    auto ite = data_.children.end();
    for ( ; it!=ite; ++it )
    {
        Node* param = *it;
        Nest::semanticCheck(param);
    }

    // Semantically check the body, if we have one
    try
    {
        if ( data_.children[1] )
            Nest::semanticCheck(data_.children[1]);

        // TODO (function): Check that all the paths return a value
    }
    catch (const exception&)
    {
        // Don't propagate errors from the body
    }
}
