#include "SparrowFrontend/StdInc.h"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Generics.hpp"
#include "SparrowFrontend/Nodes/Exp.hpp"
#include "SparrowFrontend/Mods.h"
#include "SparrowFrontend/IntMods.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "SparrowFrontend/Helpers/CommonCode.h"
#include "SparrowFrontend/Helpers/StdDef.h"
#include "SparrowFrontend/Helpers/Generics.h"
#include "SparrowFrontend/Services/IConvertService.h"
#include "SparrowFrontend/SprDebug.h"

#include "Feather/Utils/cppif/FeatherNodes.hpp"

#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/NodeHelpers.hpp"

namespace SprFrontend {

namespace {

template <typename F> inline void forEachNodeInNodeList(Feather::NodeList nodeList, F fun) {
    if (nodeList) {
        for (Nest::NodeHandle n : nodeList.children()) {
            if (!n)
                continue;

            auto nn = n.explanation();

            if (nn.kind() == nkFeatherNodeList)
                forEachNodeInNodeList(NodeList(nn), fun);
            else
                fun(n);
        }
    }
}

//! Apply the given modifier node to the given base node.
//! Recurse down if the base node is a node list or a modifiers node
void applyModifier(NodeHandle base, NodeHandle modNode) {
    Nest_Modifier* mod = nullptr;

    if (!base)
        return;

    // If the modifiers node is a node-list, apply each modifier in turn
    if (modNode.kind() == nkFeatherNodeList) {
        for (auto modNodeChild : modNode.children())
            applyModifier(base, modNodeChild);
        return;
    }

    // Recurse down if the base if is a node list or a modifiers node
    if (base.kind() == nkFeatherNodeList) {
        for (auto n : base.children())
            applyModifier(n, modNode);
        return;
    } else if (base.kind() == nkSparrowModifiersNode) {
        applyModifier(ModifiersNode(base).target(), modNode);
        return;
    }

    // Interpret the modifier expression
    Identifier modIdent = modNode.kindCast<Identifier>();
    if (modIdent) {
        StringRef name = modIdent.name();
        if (name == "public")
            mod = SprFe_getPublicMod();
        else if (name == "protected")
            mod = SprFe_getProtectedMod();
        else if (name == "private")
            mod = SprFe_getPrivateMod();
        else if (name == "ct")
            mod = SprFe_getCtMod();
        else if (name == "rt")
            mod = SprFe_getRtMod();
        else if (name == "autoCt")
            mod = SprFe_getAutoCtMod();
        else if (name == "ctGeneric")
            mod = SprFe_getCtGenericMod();
        else if (name == "convert")
            mod = SprFe_getConvertMod();
        else if (name == "noDefault")
            mod = SprFe_getNoDefaultMod();
        else if (name == "initCtor")
            mod = SprFe_getInitCtorMod();
        else if (name == "bitcopiable")
            mod = SprFe_getBitCopiableMod();
        else if (name == "autoBitcopiable")
            mod = SprFe_getAutoBitCopiableMod();
        else if (name == "macro")
            mod = SprFe_getMacroMod();
        else if (name == "noInline")
            mod = SprFe_getNoInlineMod();
    } else {
        // check for: native("name")
        if (modNode.kind() == nkSparrowExpFunApplication) {
            NodeHandle fbase = modNode.children()[0];
            NodeHandle fargs = modNode.children()[1];
            if (fbase.kind() == nkSparrowExpIdentifier &&
                    fbase.getCheckPropertyString("name") == "native") {
                // one argument: a literal
                if (fargs && fargs.kind() == nkFeatherNodeList && fargs.children().size() == 1 &&
                        fargs.children()[0].kind() == nkSparrowExpLiteral) {
                    Literal arg = Literal(fargs.children()[0]);
                    if (arg.isString()) {
                        mod = SprFe_getNativeMod(arg.dataStr());
                    }
                }
            }
        }
    }

    // If we recognized a modifier, add it to the base node; otherwise raise an error
    if (mod)
        Nest_addModifier(base, mod);
    else
        REP_ERROR(modNode.location(), "Unknown modifier found: %1%") % modNode;
}

////////////////////////////////////////////////////////////////////////////
// Helpers for VariableDecl node
//

//! The kind of variable we might have: local/global
enum VarKind {
    varLocal,
    varGlobal,
};

//! Computes the type needed for a variable/field
Type computeVarType(
        Feather::DeclNode parent, CompilationContext* ctx, NodeHandle typeNode, NodeHandle init) {
    const Location& loc = parent.location();

    Type t;

    // If a type node was given, take the type from it
    if (typeNode) {
        typeNode.setPropertyExpl(propAllowDeclExp, 1);
        t = getType(typeNode);
        if (t.hasStorage() && !Feather::isCategoryType(t)) {
            TypeWithStorage tt = TypeWithStorage(t);
            if (parent.hasProperty("addConst"))
                t = Feather::ConstType::get(tt);
        }
        if (!t)
            return {};
    } else {
        // If no type node was given, maybe a type was given directly; if so, take it
        auto givenType = parent.getPropertyType(propSprGivenType);
        t = givenType ? *givenType : nullptr;
    }

    // Should we get the type from the initialization expression?
    bool getTypeFromInit = !t;
    int numRefs = 0;
    int kind = typeKindData;
    if (t && isConceptType(t, numRefs, kind))
        getTypeFromInit = true;
    if (getTypeFromInit) {
        if (!init)
            REP_ERROR_RET(
                    nullptr, loc, "Initializer is required to deduce the type of the variable");

        if (init.computeType()) {
            // If still have a type (i.e, auto type), check for conversion
            if (t && !g_ConvertService->checkConversion(init, t))
                REP_ERROR_RET(nullptr, init.location(),
                        "Initializer of the variable (%1%) cannot be converted to variable type "
                        "(%2%)") %
                        init.type() % t;

            if (!init.type().hasStorage()) {
                REP_ERROR_RET(nullptr, init.location(), "Invalid type for variable (%1%)") %
                        init.type();
            }

            t = getAutoType(init, numRefs, kind);
        } else
            return {};
    }

    // Make sure we have the right mode for our context
    t = Feather_adjustMode(t, ctx, loc);
    return t;
}

////////////////////////////////////////////////////////////////////////////
// Helpers for DataTypeDecl node
//

//! Get the fields from the symtab of the current class.
//! In the process it might compute the type of the SprVariables
NodeVector getFields(Nest_SymTab* curSymTab) {
    // Check all the nodes registered in the children context so far to discover the fields
    NodeVector fields;
    for (Node* n : Nest_symTabAllEntries(curSymTab)) {
        if (n->nodeKind == nkFeatherDeclVar || n->nodeKind == nkSparrowDeclSprField)
            fields.push_back(n);
    }

    // Sort the fields by location - we need to add them in order
    sort(fields.begin(), fields.end(), [](Node* f1, Node* f2) {
        return Nest_compareLocations(&f1->location, &f2->location) < 0;
    });

    // Make sure we have only fields
    for (Node*& field : fields) {
        if (!Nest_computeType(field))
            field = nullptr;
        field = Nest_explanation(field);
        if (field && field->nodeKind != nkFeatherDeclVar)
            field = nullptr;
    }

    // Remove all the nulls
    fields.erase(remove_if(fields.begin(), fields.end(), [](Node* n) { return n == nullptr; }),
            fields.end());

    return fields;
}

////////////////////////////////////////////////////////////////////////////
// Helpers for SprFunction node
//

//! Helper function that creates global ctor/dtor action for static ctors/dtors
void handleStaticCtorDtor(SprFunctionDecl node, bool ctor) {
    auto parameters = node.parameters();
    auto loc = node.location();

    // Make sure we don't have any parameters
    if (parameters && !parameters.children().empty()) {
        REP_ERROR(loc, "Static constructors and destructors cannot have parameters");
        return;
    }

    // Add a global construct / destruct action call to this
    Feather::FunctionDecl expl = Feather::FunctionDecl(node.explanation());
    auto funCall = Feather::FunCallExp::create(loc, expl, NodeRange{});
    NodeHandle n = ctor ? NodeHandle(Feather::GlobalConstructAction::create(loc, funCall))
                        : NodeHandle(Feather::GlobalDestructAction::create(loc, funCall));
    n.setContext(node.context());
    if (n.semanticCheck())
        node.addAdditionalNode(n);
}

////////////////////////////////////////////////////////////////////////////
// Common code
//

//! Helper function used to create decl nodes
template <typename T>
T createDeclNode(
        const Location& loc, StringRef name, NodeRange children, bool withAccessType = true) {
    T res = Nest::createNode<T>(loc, children);
    res.setNameAndMode(name);
    if (withAccessType)
        deduceAccessType(res);
    return res;
}

//! How we want to handle children context change
enum class ContextChangeType {
    withSymTab,   //!< Create a child context with a new symtab
    ifChangeMode, //!< Only create a child context when the mode changes; keep symtab
    keepContext,  //!< Keep the same context
};

//! Common implementation for setContextForChildren
void commonSetContextForChildren(Feather::DeclNode node, ContextChangeType ctxChange) {
    // Add the node to the symtab
    Feather_addToSymTab(node);

    CompilationContext* childCtx = nullptr;
    if (!node.hasDedicatedChildrenContext()) {
        switch (ctxChange) {
        case ContextChangeType::withSymTab:
            childCtx = Nest_mkChildContextWithSymTab(node.context(), node, node.effectiveMode());
            break;
        case ContextChangeType::ifChangeMode: {
            // Create a new child compilation context if the mode has changed; otherwise stay in the
            // same context
            EvalMode curEvalMode = node.mode();
            if (curEvalMode != modeUnspecified && curEvalMode != node.context()->evalMode)
                childCtx = Nest_mkChildContext(node.context(), curEvalMode);
            break;
        }
        case ContextChangeType::keepContext:
        default:
            break;
        }
        if (childCtx)
            node.setChildrenContext(childCtx);
    }

    // Set the context for all the children
    childCtx = node.childrenContext();
    for (auto child : node.children())
        if (child)
            child.setContext(childCtx);
}

} // namespace

DEFINE_NODE_COMMON_IMPL(ModifiersNode, NodeHandle)

ModifiersNode ModifiersNode::create(const Location& loc, NodeHandle main, NodeHandle mods) {
    REQUIRE_NODE(loc, main);
    REQUIRE_NODE(loc, mods);
    return Nest::createNode<ModifiersNode>(loc, NodeRange({main, mods}));
}

void ModifiersNode::setContextForChildrenImpl(ModifiersNode node) {
    NodeHandle targetNode = node.target();
    NodeHandle modifierNodes = node.mods();

    if (modifierNodes) {
        // Set the context of the modifiers
        modifierNodes.setContext(node.context());
        // Interpret the modifiers here - as much as possible
        applyModifier(targetNode, modifierNodes);
    }

    // Set the context for the target node
    if (targetNode)
        targetNode.setContext(node.context());
}

Type ModifiersNode::computeTypeImpl(ModifiersNode node) { return node.target().computeType(); }
NodeHandle ModifiersNode::semanticCheckImpl(ModifiersNode node) { return node.target(); }

DEFINE_NODE_COMMON_IMPL(UsingDecl, DeclNode)

UsingDecl UsingDecl::create(const Location& loc, StringRef name, NodeHandle usedNode) {
    REQUIRE_NODE(loc, usedNode);
    return createDeclNode<UsingDecl>(loc, name, NodeRange({usedNode}));
}

void UsingDecl::setContextForChildrenImpl(UsingDecl node) {
    if (node.name())
        Feather_addToSymTab(node);
    else
        Nest_symTabAddToCheckNode(node.context()->currentSymTab, node);

    DeclNode::setContextForChildrenImpl(node);
}

Type UsingDecl::computeTypeImpl(UsingDecl node) {
    NodeHandle usedNode = node.usedNode();
    StringRef alias = node.name();

    // Compile the used node
    usedNode.setPropertyExpl(propAllowDeclExp, 1);
    if (!usedNode.semanticCheck())
        return {};

    if (!alias) {
        // Make sure that this node refers to one or more declaration
        Node* baseExp;
        NodeVector decls = getDeclsFromNode(usedNode, baseExp);
        if (decls.empty() && !node.hasProperty(propNoWarnIfNoDeclFound))
            REP_WARNING(usedNode.location(), "No declarations can be found for using");

        // Add references in the current symbol tab
        for (Node* decl : decls) {
            if (!isProtected(decl))
                Nest_symTabEnter(node.context()->currentSymTab, Feather_getName(decl).begin, decl);
        }
    } else {
        // We already added this node to the current sym tab, in setContextForChildren()

        // If this points to another decl (and only one), set the resulting decl for this node.
        DeclExp usedDeclExp = usedNode.kindCast<DeclExp>();
        if (usedDeclExp && usedDeclExp.referredDecls().size() == 1) {
            NodeHandle decl = usedDeclExp.referredDecls()[0];
            node.setProperty(propResultingDecl, decl);
        }

        // If this is a CT expression, eval it
        // We store the result back in our used node
        usedNode.semanticCheck();
        if (usedNode.type() && usedNode.type().hasStorage() && usedNode.type().mode() == modeCt) {
            node.childrenM()[0] = Nest_ctEval(usedNode);
        }
    }

    // This expands to Nop
    Feather::Nop expl = Feather::Nop::create(node.location());
    node.handle->explanation = expl;
    expl.setContext(node.context());
    if (!expl.semanticCheck())
        return {};
    return expl.type();
}

NodeHandle UsingDecl::semanticCheckImpl(UsingDecl node) {
    return node.computeType() ? node.explanation() : NodeHandle();
}

DEFINE_NODE_COMMON_IMPL(PackageDecl, DeclNode)

PackageDecl PackageDecl::create(const Location& loc, StringRef name, NodeHandle body,
        NodeList params, NodeHandle ifClause) {
    REQUIRE_NODE(loc, body);
    return createDeclNode<PackageDecl>(loc, name, NodeRange({body, params, ifClause}));
}

void PackageDecl::setContextForChildrenImpl(PackageDecl node) {
    commonSetContextForChildren(node, ContextChangeType::withSymTab);
}

Type PackageDecl::computeTypeImpl(PackageDecl node) {
    NodeHandle body = node.body();
    NodeList parameters = node.parameters();
    NodeHandle ifClause = node.ifClause();

    // Is this a generic?
    // TODO (nodes): Does it make sense to specialize this from the beginning?
    if (parameters && !parameters.children().empty()) {
        auto generic = GenericPackage::create(node, parameters, ifClause);
        node.setProperty(propResultingDecl, generic);
        generic.setContext(node.context());
        if (!generic.semanticCheck())
            return {};
        node.setExplanation(generic);
        return generic.type();
    }
    if (ifClause)
        REP_ERROR_RET(Type{}, node.location(),
                "If clauses must be applied only to generics; this is not a generic package");

    // This can be computed without checking the children
    auto resType = Feather::VoidType::get(modeCt);
    node.setType(resType);

    // Compute the type for the body
    if (body && !body.computeType())
        return {};
    node.setExplanation(body);
    if (body)
        checkForAllowedNamespaceChildren(body);

    return resType;
}

NodeHandle PackageDecl::semanticCheckImpl(PackageDecl node) {
    // Ensure type is computed first
    if (!node.computeType())
        return {};

    // If this is a generic, there is nothing we should do here
    if (node.parameters())
        return node.explanation();

    return node.body().semanticCheck();
}

DEFINE_NODE_COMMON_IMPL(VariableDecl, DeclNode)

VariableDecl VariableDecl::createConst(
        const Location& loc, StringRef name, NodeHandle typeNode, NodeHandle init) {
    if (!typeNode && !init)
        REP_ERROR(loc, "Cannot create variable without a type and without an initializer");
    auto res = createDeclNode<VariableDecl>(loc, name, NodeRange({typeNode, init}));
    res.setProperty("addConst", 1);
    return res;
}
VariableDecl VariableDecl::createMut(
        const Location& loc, StringRef name, NodeHandle typeNode, NodeHandle init) {
    if (!typeNode && !init)
        REP_ERROR(loc, "Cannot create variable without a type and without an initializer");
    return createDeclNode<VariableDecl>(loc, name, NodeRange({typeNode, init}));
}

VariableDecl VariableDecl::create(
        const Location& loc, StringRef name, TypeWithStorage type, NodeHandle init) {
    CHECK(loc, type);
    auto res = createDeclNode<VariableDecl>(loc, name, NodeRange({nullptr, init}));
    res.setProperty(propSprGivenType, type);
    return res;
}

void VariableDecl::setContextForChildrenImpl(VariableDecl node) {
    commonSetContextForChildren(node, ContextChangeType::ifChangeMode);
}

Type VariableDecl::computeTypeImpl(VariableDecl node) {
    NodeHandle typeNode = node.typeNode();
    NodeHandle init = node.init();
    Location loc = node.location();

    // Check the kind of the variable (local, global, field)
    NodeHandle parentFun = Feather_getParentFun(node.context());
    VarKind varKind = parentFun ? varLocal : varGlobal;

    // Get the type of the variable
    Type t = computeVarType(node, node.childrenContext(), typeNode, init);
    if (!t && !t.hasStorage())
        return {};

    // If the type of the variable indicates a variable that can only be CT, change the evalMode
    if (t.mode() == modeCt)
        node.setMode(modeCt);

    // Create the resulting var
    auto resultingVar =
            Feather::VarDecl::create(loc, node.name(), Feather::TypeNode::create(loc, t));
    resultingVar.setMode(node.effectiveMode());
    Feather_setShouldAddToSymTab(resultingVar, 0);
    node.setProperty(propResultingDecl, resultingVar);

    // Set the context and compute the type for the resulting var
    resultingVar.setContext(node.childrenContext());
    if (!resultingVar.computeType())
        return {};

    // If this is a CT variable in a non-ct function, make this a global variable
    if (varKind == varLocal && node.context()->evalMode == modeRt && t.mode() == modeCt)
        varKind = varGlobal;

    bool isRef = Feather::removeCategoryIfPresent(t).numReferences() > 0;

    // Generate the initialization and destruction calls
    NodeHandle ctorCall;
    NodeHandle dtorCall;
    NodeHandle varRef;
    if (Feather::isDataLikeType(t) && (init || !isRef)) {
        ASSERT(resultingVar.type());

        // Create a var-ref object to refer to the variable to be initialized/destructed.
        NodeHandle varRef = Feather::VarRefExp::create(loc, resultingVar);

        // If the variable is const, cast the constness away for initialization & destruction
        if (resultingVar.type().kind() == Feather_getConstTypeKind()) {
            TypeWithStorage t = Feather::ConstType(resultingVar.type()).base();
            t = Feather::MutableType::get(t);
            auto typeNode = Feather::TypeNode::create(loc, t);
            varRef = Feather::BitcastExp::create(loc, typeNode, varRef);
        }

        varRef.setContext(node.childrenContext());

        if (!isRef) {
            // Create ctor and dtor
            ctorCall = createCtorCall(loc, node.childrenContext(), varRef, init);
            if (resultingVar.type().mode() != modeCt)
                dtorCall = createDtorCall(loc, node.childrenContext(), varRef);
        } else if (init) { // Reference initialization
            // Create an assignment operator
            ctorCall = mkOperatorCall(loc, varRef, StringRef(":="), init);
        }
    }

    // Set the explanation of this node; take into consideration the ctor and dtor calls
    NodeHandle expl;
    if (varKind == varLocal) {
        // For local variables, add the ctor & dtor actions in the node list, and make this as
        // explanation
        dtorCall = dtorCall ? Feather::ScopeDestructAction::create(loc, dtorCall) : nullptr;
        expl = Feather::NodeList::create(loc, NodeRange({resultingVar, ctorCall, dtorCall}), true);
    } else {
        // For global variables, wrap ctor & dtor nodes in global ctor/dtor actions
        if (ctorCall)
            ctorCall = Feather::GlobalConstructAction::create(loc, ctorCall);
        if (dtorCall)
            dtorCall = Feather::GlobalDestructAction::create(loc, dtorCall);

        expl = Feather::NodeList::create(loc, fromIniList({ctorCall, dtorCall}), true);

        // This is a global var: don't include it into the function scope
        // Add it as an additional (top-level) node to the result
        expl.addAdditionalNode(resultingVar);
    }
    ASSERT(expl);

    expl.setContext(node.childrenContext());
    if (!expl.computeType())
        return {};
    node.setType(expl.type());
    node.setExplanation(expl);

    node.setProperty("spr.resultingVar", resultingVar);

    return node.type();
}

NodeHandle VariableDecl::semanticCheckImpl(VariableDecl node) {
    if (!node.computeType())
        return {};

    // Semantically check the resulting variable and explanation
    if (!node.resultingVar().semanticCheck())
        return {};
    return node.explanation().semanticCheck();
}

DEFINE_NODE_COMMON_IMPL(DataTypeDecl, DeclNode)

DataTypeDecl DataTypeDecl::create(const Location& loc, StringRef name, NodeList parameters,
        NodeHandle underlyingData, NodeHandle ifClause, NodeList body) {
    if (underlyingData && body)
        REP_ERROR(loc, "Cannot specify both underlying data and a body to a datatype declaration");

    // If we have an underlying data, create a body out of it
    if (underlyingData) {
        auto innerVar = FieldDecl::create(loc, StringRef("data"), underlyingData, nullptr);
        body = Feather::NodeList::create(loc, NodeRange({innerVar}));
    }

    auto res = createDeclNode<DataTypeDecl>(loc, name, NodeRange({parameters, body, ifClause}));
    if (underlyingData)
        res.setProperty(propGenerateInitCtor, 1);
    return res;
}

void DataTypeDecl::setContextForChildrenImpl(DataTypeDecl node) {
    commonSetContextForChildren(node, ContextChangeType::withSymTab);
}

Type DataTypeDecl::computeTypeImpl(DataTypeDecl node) {
    NodeList parameters = node.parameters();
    NodeList body = node.body();
    NodeHandle ifClause = node.ifClause();
    const Location& loc = node.location();

    // Is this a generic?
    if (parameters && !parameters.children().empty()) {
        auto generic = GenericDatatype::create(node, parameters, ifClause);
        node.setProperty(propResultingDecl, generic);
        generic.setContext(node.context());
        if (!generic.semanticCheck())
            return {};
        node.setExplanation(generic);
        return generic.type();
    }
    if (ifClause)
        REP_ERROR_RET(nullptr, loc,
                "If clauses must be applied only to generics; this is not a generic datatype");

    // Default class members
    if (!node.hasProperty(propNoDefault))
        node.addModifier(SprFe_getClassMembersIntMod());

    Feather::StructDecl resultingStruct;

    // Special case for Type class; re-use the existing StdDef class
    StringRef nativeName = node.getPropertyStringDeref(propNativeName);
    if (nativeName == "Type")
        resultingStruct = StdDef::clsType;

    // Create the resulting Feather.Class object
    if (!resultingStruct) {
        resultingStruct = Feather::StructDecl::create(loc, node.name(), NodeRange{});
        copyAccessType(resultingStruct, node);
    }
    Feather_setShouldAddToSymTab(resultingStruct, 0);

    // Copy the "convert", "native" and "description" properties to the resulting class
    if (node.hasProperty(propConvert))
        resultingStruct.setProperty(propConvert, 1);
    if (nativeName)
        resultingStruct.setProperty(propNativeName, nativeName);
    if (node.hasProperty(propBitCopiable))
        resultingStruct.setProperty(propBitCopiable, 1);
    auto description = node.getPropertyString(propDescription);
    if (description)
        resultingStruct.setProperty(propDescription, *description);

    EvalMode evalMode = node.effectiveMode();
    resultingStruct.setMode(evalMode);
    resultingStruct.setChildrenContext(node.childrenContext());
    resultingStruct.setContext(node.context());
    node.setProperty(propResultingDecl, resultingStruct);

    node.setExplanation(resultingStruct);

    // Check for Std classes
    checkStdClass(resultingStruct);

    // We now have a type - from now on we can safely compute the types of the children
    node.setType(Feather::DataType::get(resultingStruct, 0, evalMode));

    // Get the fields from the current datatype
    NodeVector fields = getFields(node.childrenContext()->currentSymTab);
    resultingStruct.addChildren(all(fields));

    // Check for autoBitcopiable
    if (node.hasProperty(propAutoBitCopiable)) {
        bool allAreBitcopiable = true;
        for (auto f : fields) {
            ASSERT(f->type);
            allAreBitcopiable = allAreBitcopiable && isBitCopiable(f->type);
        }
        if (allAreBitcopiable)
            resultingStruct.setProperty(propBitCopiable, 1);
    }

    // Check all the members
    if (body) {
        body.computeType(); // Ignore local errors
        checkForAllowedNamespaceChildren(body, true);
    }

    // Add all the non-fields members (using decls) from the body and add them as additional nodes
    forEachNodeInNodeList(body, [&](NodeHandle member) -> void {
        auto p = member.explanation();
        if (p.kind() != nkFeatherDeclVar) {
            // Using decls
            node.addAdditionalNode(member);
        }
    });

    // Compute the type for the basic class
    if (!resultingStruct.computeType())
        return {};
    return node.type();
}

NodeHandle DataTypeDecl::semanticCheckImpl(DataTypeDecl node) {
    if (!node.computeType())
        return {};

    if (!node.explanation().semanticCheck())
        return {};

    if (node.explanation().kind() != nkFeatherDeclClass)
        return node.explanation(); // This should be a generic; there is nothing else to do here

    // Semantic check the body
    auto body = node.body();
    if (body)
        body.semanticCheck(); // Ignore possible failures
    return node.explanation();
}

DEFINE_NODE_COMMON_IMPL(FieldDecl, DeclNode)

FieldDecl FieldDecl::create(
        const Location& loc, StringRef name, NodeHandle typeNode, NodeHandle init) {
    if (!typeNode && !init)
        REP_ERROR(loc, "Cannot create field without a type and without an initializer");
    return createDeclNode<FieldDecl>(loc, name, NodeRange({typeNode, init}), false);
}

void FieldDecl::setContextForChildrenImpl(FieldDecl node) {
    commonSetContextForChildren(node, ContextChangeType::ifChangeMode);
}

Type FieldDecl::computeTypeImpl(FieldDecl node) {
    NodeHandle typeNode = node.typeNode();
    NodeHandle init = node.init();
    const Location& loc = node.location();

    // Get the type of the variable
    Type t = computeVarType(node, node.childrenContext(), typeNode, init);
    if (!t)
        return {};

    int origTypeKind = -1;
    if (Feather::isCategoryType(t)) {
        origTypeKind = t.kind();
        t = Feather::removeCategoryIfPresent(t);
    }

    // If the type of the variable indicates a variable that can only be CT, change the evalMode
    if (t.mode() == modeCt)
        node.setMode(modeCt);

    // Create the resulting var
    auto resultingVar =
            Feather::VarDecl::create(loc, node.name(), Feather::TypeNode::create(loc, t));
    resultingVar.setMode(node.effectiveMode());
    Feather_setShouldAddToSymTab(resultingVar, 0);
    node.setProperty(propResultingDecl, resultingVar);
    if (origTypeKind >= 0)
        resultingVar.setProperty(propSprOrigCat, origTypeKind);

    // Set the context and compute the type for the resulting var
    resultingVar.setContext(node.childrenContext());
    if (!resultingVar.computeType())
        return {};

    if (init)
        resultingVar.setProperty(propVarInit, init);

    // Set the explanation of this node
    resultingVar.setContext(node.childrenContext());
    auto resType = resultingVar.computeType();
    if (!resType)
        return {};
    node.setType(resType);
    node.setExplanation(resultingVar);

    return resType;
}

NodeHandle FieldDecl::semanticCheckImpl(FieldDecl node) {
    if (!node.computeType())
        return {};
    return node.explanation().semanticCheck();
}

DEFINE_NODE_COMMON_IMPL(SprFunctionDecl, DeclNode)

SprFunctionDecl SprFunctionDecl::create(const Location& loc, StringRef name, NodeList parameters,
        NodeHandle returnType, NodeHandle body, NodeHandle ifClause) {
    return createDeclNode<SprFunctionDecl>(
            loc, name, NodeRange({parameters, returnType, body, ifClause}));
}

void SprFunctionDecl::setContextForChildrenImpl(SprFunctionDecl node) {
    commonSetContextForChildren(node, ContextChangeType::withSymTab);
}

Type SprFunctionDecl::computeTypeImpl(SprFunctionDecl node) {
    NodeList parameters = node.parameters();
    NodeHandle returnType = node.returnType();
    NodeHandle body = node.body();
    NodeHandle ifClause = node.ifClause();
    const Location& loc = node.location();

    if (returnType)
        returnType.setPropertyExpl(propAllowDeclExp, 1);

    // Does this function have an implicit 'this' arg?
    int thisParamIdx = -1;
    if (parameters) {
        for (int i = 0; i < parameters.children().size(); i++) {
            Node* param = parameters.children()[i];
            if (!param)
                continue;
            StringRef name = DeclNode(param).name();
            if (name == "this") {
                thisParamIdx = i;
                break;
            }
        }
    }
    if (thisParamIdx >= 0)
        node.setProperty(propThisParamIdx, thisParamIdx);

    // Is this a generic?
    if (parameters) {
        NodeHandle generic = checkCreateGenericFun(node, parameters.children(), ifClause);
        if (generic) {
            node.setExplanation(generic);
            if (!generic.computeType())
                return {};
            node.setProperty(propResultingDecl, generic);
            return generic.type();
        }
    }
    if (ifClause)
        REP_ERROR_RET(nullptr, loc,
                "If clauses must be applied only to generics; this is not a generic function");

    StringRef funName = node.name();

    // Special modifier for ctors & dtors
    if ((thisParamIdx >= 0) && !node.hasProperty(propNoDefault)) {
        if (funName == "ctor")
            node.addModifier(SprFe_getCtorMembersIntMod());
        if (funName == "dtor")
            node.addModifier(SprFe_getDtorMembersIntMod());
    }

    EvalMode thisEvalMode = node.effectiveMode();

    // Create the resulting function object
    Feather::FunctionDecl resultingFun =
            Feather::FunctionDecl::create(loc, funName, nullptr, NodeRange{}, body);
    Feather_setShouldAddToSymTab(resultingFun, 0);
    copyAccessType(resultingFun, node);

    // Copy the "native" and the "autoCt" properties
    StringRef nativeName = node.getPropertyStringDeref(propNativeName);

    if (nativeName)
        resultingFun.setProperty(propNativeName, nativeName);
    if (node.hasProperty(propAutoCt))
        resultingFun.setProperty(propAutoCt, 1);
    if (node.hasProperty(propNoInline))
        resultingFun.setProperty(propNoInline, 1);

    resultingFun.setMode(thisEvalMode);
    resultingFun.setChildrenContext(node.childrenContext());
    resultingFun.setContext(node.context());
    node.setProperty(propResultingDecl, resultingFun);
    copyOverloadPrio(node, resultingFun);

    // Compute the types of the parameters first
    if (parameters && !parameters.computeType())
        return {};

    // Add the actual specified parameters
    if (parameters) {
        for (NodeHandle n : parameters.children()) {
            if (!n)
                REP_ERROR_RET(nullptr, parameters.location(), "Invalid node as parameter");
            if (n.hasError())
                return {};
            if (n.explanation().kind() != nkFeatherDeclVar) {
                REP_INTERNAL(n.location(), "Trouble converting %1% to a Feather parameter") %
                        Nest_toStringEx(n);
            }

            resultingFun.addParameter(DeclNode(n));
        }
    }

    // If we found an explicit 'this' parameter, mark this as a property of the resulting function
    if (thisParamIdx >= 0) {
        NodeHandle thisParam = parameters.children()[thisParamIdx];
        ASSERT(thisParam);
        resultingFun.setProperty(propThisParamType, thisParam.type());
    }

    // The explanation may be needed while computing the return type
    // E.g., when we are using implicit 'this' for expressions that would be
    // used to compute the result type (fun name = expr)
    node.setExplanation(resultingFun);

    // Compute the type of the return type node
    // We do this after the parameters, as the computation of the result might require access to the
    // parameters
    Type resType = returnType ? getType(returnType) : Feather::VoidType::get(thisEvalMode);
    if (!resType)
        REP_INTERNAL(loc, "Cannot compute the function resulting type");
    resType = resType.changeMode(thisEvalMode, loc);

    // If the result is a non-reference, non bitcopiable datatype, not basic numeric,
    // and our function is not native, add result parameter; otherwise, normal result
    bool preserveRetAbi = !resType.hasStorage();
    preserveRetAbi = preserveRetAbi || resType.numReferences() > 0;
    preserveRetAbi = preserveRetAbi || isBitCopiable(resType);
    preserveRetAbi = preserveRetAbi || (nativeName && nativeName != "$funptr");
    if (!preserveRetAbi) {
        ASSERT(returnType);
        const Location& retLoc = returnType.location();
        auto resParamType = Feather::MutableType::get(TypeWithStorage(resType));
        auto resParam = Feather::VarDecl::create(
                retLoc, StringRef("_result"), Feather::TypeNode::create(retLoc, resParamType));
        resParam.setContext(node.childrenContext());
        resultingFun.addParameter(resParam, true);
        resultingFun.setProperty(propResultParam, resParam);
        resultingFun.setResultType(
                Feather::TypeNode::create(retLoc, Feather::VoidType::get(thisEvalMode)));
    } else
        resultingFun.setResultType(Feather::TypeNode::create(loc, resType));

    // Now we can actually compute the type of the resulting function
    node.setType(resultingFun.computeType());

    // Check for Std functions
    checkStdFunction(node);

    return node.type();
}

NodeHandle SprFunctionDecl::semanticCheckImpl(SprFunctionDecl node) {
    if (!node.computeType())
        return {};
    NodeHandle resultingFun = node.explanation();
    NodeList parameters = node.parameters();

    ASSERT(resultingFun);
    if (!resultingFun.semanticCheck())
        return {};

    // Check for static ctors & dtors
    // A static ctor/ctor has no parameters (i.e., the 'this' parameter)
    if (resultingFun && (!parameters || parameters.children().empty())) {
        StringRef funName = node.name();

        if (funName == "ctor")
            handleStaticCtorDtor(node, true);
        else if (funName == "dtor")
            handleStaticCtorDtor(node, false);
    }
    return resultingFun;
}

DEFINE_NODE_COMMON_IMPL(ParameterDecl, DeclNode)

ParameterDecl ParameterDecl::create(
        const Location& loc, StringRef name, NodeHandle typeNode, NodeHandle init) {
    return createDeclNode<ParameterDecl>(loc, name, NodeRange({typeNode, init}), false);
}

ParameterDecl ParameterDecl::create(
        const Location& loc, StringRef name, TypeWithStorage type, NodeHandle init) {
    auto res = createDeclNode<ParameterDecl>(loc, name, NodeRange({NodeHandle{}, init}), false);
    res.setProperty(propSprGivenType, type);
    return res;
}

ParameterDecl ParameterDecl::create(const Location& loc, StringRef name) {
    auto typeNode = Identifier::create(loc, StringRef("AnyType"));
    return create(loc, name, typeNode, NodeHandle{});
}

void ParameterDecl::setContextForChildrenImpl(ParameterDecl node) {
    commonSetContextForChildren(node, ContextChangeType::keepContext);
}

Type ParameterDecl::computeTypeImpl(ParameterDecl node) {
    NodeHandle typeNode = node.typeNode();
    const Location& loc = node.location();

    if (typeNode)
        typeNode.setPropertyExpl(propAllowDeclExp, 1);

    auto givenType = node.getPropertyType(propSprGivenType);
    Type t = givenType ? *givenType : getType(typeNode);
    if (!t)
        return {};

    // Should with make this parameter const?
    if (shouldMakeParamConst(t) && !node.hasProperty(propNoAutoConst))
        t = Feather::ConstType::get(TypeWithStorage(t));

    auto resultingParam =
            Feather::VarDecl::create(loc, node.name(), Feather::TypeNode::create(loc, t));
    resultingParam.setMode(node.effectiveMode());
    Feather_setShouldAddToSymTab(resultingParam, 0);
    resultingParam.setContext(node.context());
    if (!resultingParam.computeType())
        return {};
    node.setProperty(propResultingDecl, resultingParam);
    node.setExplanation(resultingParam);
    return resultingParam.type();
}

NodeHandle ParameterDecl::semanticCheckImpl(ParameterDecl node) {
    if (!node.computeType())
        return {};

    if (!node.explanation().semanticCheck())
        return {};

    auto init = node.init();
    if (init && !init.semanticCheck())
        return {};
    return node.explanation();
}

DEFINE_NODE_COMMON_IMPL(ConceptDecl, DeclNode)

ConceptDecl ConceptDecl::create(const Location& loc, StringRef name, StringRef paramName,
        NodeHandle baseConcept, NodeHandle ifClause) {
    auto res = createDeclNode<ConceptDecl>(loc, name, NodeRange({baseConcept, ifClause, nullptr}));
    res.setProperty("spr.paramName", paramName);
    return res;
}

StringRef ConceptDecl::paramName() const { return getCheckPropertyString("spr.paramName"); }

InstantiationsSet ConceptDecl::instantiationsSet() const { return {children()[2]}; };

void ConceptDecl::setContextForChildrenImpl(ConceptDecl node) {
    commonSetContextForChildren(node, ContextChangeType::withSymTab);
}

NodeHandle ConceptDecl::semanticCheckImpl(ConceptDecl node) {
    NodeHandle baseConcept = node.baseConcept();
    NodeHandle ifClause = node.ifClause();
    StringRef paramName = node.paramName();
    const Location& loc = node.location();

    // Compile the base concept node; make sure it's ct
    if (baseConcept) {
        if (!baseConcept.semanticCheck())
            return {};
        if (!Feather_isCt(baseConcept))
            REP_ERROR_RET(nullptr, baseConcept.location(),
                    "Base concept type needs to be compile-time (type=%1%)") %
                    baseConcept.type();
    }

    NodeHandle param = baseConcept ? ParameterDecl::create(loc, paramName, baseConcept)
                                   : ParameterDecl::create(loc, paramName, ConceptType::get());
    param.setContext(node.childrenContext());
    if (!param.computeType()) // But not semanticCheck, as it will complain of instantiating a
                              // var of type auto
        return {};

    // Create the instantiation set node, to store which types fulfill this concept
    node.childrenM()[2] = InstantiationsSet::create(node, fromIniList({param}), ifClause);
    return Feather::Nop::create(loc);
}

} // namespace SprFrontend
