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
    children_.insert(children_.begin(), body);
    children_.insert(children_.begin(), resultType);

    setName(this, move(name));
    setProperty("callConvention", (int) callConv);
}

void Function::addParameter(DynNode* parameter, bool first)
{
    if ( parameter->explanation()->nodeKind() != nkFeatherDeclVar )
        REP_INTERNAL(parameter->location(), "DynNode %1% must be a parameter") % parameter;

    ASSERT(children_.size() >= 2);
    if ( first )
        children_.insert(children_.begin()+2, parameter);
    else
        children_.push_back(parameter);
}

void Function::setResultType(DynNode* resultType)
{
    ASSERT(children_.size() >= 2);
    children_[0] = resultType;
    resultType->setContext(childrenContext_);
}

void Function::setBody(DynNode* body)
{
    ASSERT(children_.size() >= 2);
    children_[1] = body;
}

size_t Function::numParameters() const
{
    return children_.size()-2;
}
DynNode* Function::getParameter(size_t idx) const
{
    return children_[idx+2];
}

TypeRef Function::resultType() const
{
    ASSERT(children_.size() >= 2);
    return children_[0]->type();
}

DynNode* Function::body() const
{
    ASSERT(children_.size() >= 2);
    return children_[1];
}

CallConvention Function::callConvention() const
{
    return (CallConvention) getCheckPropertyInt("callConvention");
}


void Function::dump(ostream& os) const
{
    os << endl;
    os << "fun(\"" << getName(this) << "\"";
    auto it = children_.begin()+2;
    auto ite = children_.end();
    for ( ; it!=ite; ++it )
    {
        os << ", " << *it;
    }
    os << "): " << resultType() << " {" << endl;
    os << children_[1];
    os << "}";
}

string Function::toString() const
{
    ostringstream oss;
    oss << getName(this);
    if ( type_ )
    {
        oss << '(';
        bool hasResultParam = hasProperty(propResultParam);
        size_t startIdx = hasResultParam ? 3 : 2;
        for ( size_t i=startIdx; i<children_.size(); ++i )
        {
            if ( i > startIdx )
                oss << ", ";
            oss << children_[i]->type();
        }
        oss << "): " << (hasResultParam ? removeRef(children_[2]->type()) : resultType());
    }
    return oss.str();
//    return type_ ? getName(this) + type_->toString() : getName(this);
}

void Function::doSetContextForChildren()
{
    // If we don't have a children context, create one
    if ( !childrenContext_ )
        childrenContext_ = context_->createChildContext(this, effectiveEvalMode(this));

    DynNode::doSetContextForChildren();
    
    addToSymTab(this);
}

void Function::doComputeType()
{
    if ( getName(this).empty() )
        REP_ERROR(location_, "No name given to function declaration");

    // We must have a result type
    DynNode* resultType = children_[0];
    resultType->computeType();
    TypeRef resType = resultType->type();
    if ( !resType )
        REP_ERROR(location_, "No result type given to function %1%") % getName(this);

    // Compute the type for all the parameters
    auto it = children_.begin()+2;
    auto ite = children_.end();
    for ( ; it!=ite; ++it )
    {
        DynNode* param = *it;
        if ( !param )
            REP_ERROR(location_, "Invalid param");
	    param->computeType();
    }

    // Set the type for this node
    vector<TypeRef> subTypes;
    subTypes.reserve(children_.size()-1);
    subTypes.push_back(resType);
    it = children_.begin()+2;
    for ( ; it!=ite; ++it )
    {
        DynNode* param = *it;
        subTypes.push_back(param->type());
    }
    type_ = getFunctionType(&subTypes[0], subTypes.size(), effectiveEvalMode(this));
}

void Function::doSemanticCheck()
{
    // Make sure the type is computed
    computeType();

    // Semantically check all the parameters
    auto it = children_.begin()+2;
    auto ite = children_.end();
    for ( ; it!=ite; ++it )
    {
        DynNode* param = *it;
        param->semanticCheck();
    }

    // Semantically check the body, if we have one
    try
    {
        if ( children_[1] )
            children_[1]->semanticCheck();

        // TODO (function): Check that all the paths return a value
    }
    catch (const exception&)
    {
        // Don't propagate errors from the body
    }
}
