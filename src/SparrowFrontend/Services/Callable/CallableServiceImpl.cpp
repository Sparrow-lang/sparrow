#include <StdInc.h>
#include "SparrowFrontend/Services/Callable/CallableServiceImpl.h"
#include "SparrowFrontend/Services/Callable/FunctionCallable.h"
#include "SparrowFrontend/Services/Callable/GenericFunctionCallable.h"
#include "SparrowFrontend/Services/Callable/GenericDatatypeCallable.h"
#include "SparrowFrontend/Services/Callable/GenericPackageCallable.h"
#include "SparrowFrontend/Services/Callable/ConceptCallable.h"
#include "SparrowFrontend/Nodes/Generics.hpp"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"

namespace SprFrontend {

namespace {
//! Checks if the given decl satisfies the predicate
//! If the predicate is empty, the decl always satisfies it
bool predIsSatisfied(NodeHandle decl, const std::function<bool(NodeHandle)>& pred) {
    return !pred || pred(decl);
}

//! Get the class-ctor callables corresponding to the given class
void getClassCtorCallables(Feather::StructDecl structDecl, EvalMode evalMode, Callables& res,
        const std::function<bool(NodeHandle)>& pred, const char* ctorName) {
    // Search for the ctors associated with the class
    auto decls = getClassAssociatedDecls(structDecl, ctorName);

    evalMode = Feather_combineMode(structDecl.effectiveMode(), evalMode);
    if (!structDecl.computeType())
        return;

    // Get the type of the temporary variable created when constructing the datatype
    TypeWithStorage implicitArgType = structDecl.type();
    if (evalMode != modeRt)
        implicitArgType = implicitArgType.changeMode(evalMode, structDecl.location());

    res.callables_.reserve(res.size() + Nest_nodeArraySize(decls));
    for (NodeHandle decl : decls) {
        if (!decl.computeType())
            continue;
        auto funDecl = decl.explanation().kindCast<Feather::FunctionDecl>();
        if (funDecl && predIsSatisfied(decl, pred))
            res.callables_.push_back(new FunctionCallable(funDecl, implicitArgType));

        NodeHandle resDecl = resultingDecl(decl);
        auto genFun = resDecl.kindCast<GenericFunction>();
        if (genFun && predIsSatisfied(decl, pred))
            res.callables_.push_back(new GenericFunctionCallable(genFun, implicitArgType));
        auto genDatatype = resDecl.kindCast<GenericDatatype>();
        if (genDatatype && predIsSatisfied(decl, pred))
            res.callables_.push_back(new GenericDatatypeCallable(genDatatype));
    }
    Nest_freeNodeArray(decls);
}
} // namespace

Callables CallableServiceImpl::getCallables(NodeRange decls, EvalMode evalMode,
        const std::function<bool(NodeHandle)>& pred, const char* ctorName) {
    Callables res;
    auto declsEx = expandDecls(decls, nullptr);

    for (NodeHandle d1 : declsEx) {
        NodeHandle node = d1;

        // If we have a resolved decl, get the callable for it
        if (node) {
            if (!node.computeType())
                continue;

            NodeHandle decl = resultingDecl(node);
            if (!decl)
                continue;

            // Is this a normal function call?
            auto funDecl = decl.kindCast<Feather::FunctionDecl>();
            if (funDecl && predIsSatisfied(funDecl, pred))
                res.callables_.push_back(new FunctionCallable(funDecl));

            // Is this a generic?
            auto genFun = decl.kindCast<GenericFunction>();
            if (genFun && predIsSatisfied(genFun, pred))
                res.callables_.push_back(new GenericFunctionCallable(genFun));
            auto genDatatype = decl.kindCast<GenericDatatype>();
            if (genDatatype && predIsSatisfied(genDatatype, pred))
                res.callables_.push_back(new GenericDatatypeCallable(genDatatype));
            auto genPackage = decl.kindCast<GenericPackage>();
            if (genPackage && predIsSatisfied(genPackage, pred))
                res.callables_.push_back(new GenericPackageCallable(genPackage));

            // Is this a concept?
            auto concept = decl.kindCast<ConceptDecl>();
            if (concept && predIsSatisfied(concept, pred))
                res.callables_.push_back(new ConceptCallable(concept));

            // Is this a temporary object creation?
            auto structDecl = decl.kindCast<Feather::StructDecl>();
            if (structDecl) {
                getClassCtorCallables(structDecl, evalMode, res, pred, ctorName);
            }
        }
    }
    return res;
}

} // namespace SprFrontend
