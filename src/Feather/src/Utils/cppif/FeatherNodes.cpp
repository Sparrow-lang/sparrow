#include "Feather/src/StdInc.h"
#include "Feather/Utils/cppif/FeatherNodes.hpp"
#include "Feather/src/FeatherNodeCommonsCpp.h"
#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.h"
#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"

namespace Feather {

namespace {

const char* propResultVoid = "nodeList.resultVoid";

//! Returns the number of references from the given type. Returns -1 if not storage type.
int numRefs(Type t) { return t.hasStorage() ? TypeWithStorage(t).numReferences() : -1; }

void CtProcessMod_afterSemanticCheck(Nest_Modifier* mod, Node* node) { Nest_ctProcess(node); }
Nest_Modifier ctProcessMod = {modTypeAfterSemanticCheck, &CtProcessMod_afterSemanticCheck};

/// Checks if the given node has some possible instructions
/// Does not need to be 100% accurate, can be conservative
bool canHaveInstructions(NodeHandle node) {
    if (!node)
        return false;
    switch (node.kind() - Feather_getFirstFeatherNodeKind()) {
    case nkRelFeatherNodeList:
    case nkRelFeatherLocalSpace:
        for (auto n : node.children()) {
            if (canHaveInstructions(n.explanation()))
                return true;
        }
        return false;
    case nkRelFeatherExpFunCall: {
        FunctionDecl fun = FunCallExp(node).funDecl();
        if (!fun.hasProperty(propEmptyBody))
            return true;
        // Check the args for side-effects
        for (auto arg : node.children()) {
            if (canHaveInstructions(arg.explanation()))
                return true;
        }
        return false;
    }
    // These can have side effects if the first child has side effects
    case nkRelFeatherExpBitcast:
        return canHaveInstructions(BitcastExp(node).expression().explanation());
    case nkRelFeatherExpMemLoad:
        return canHaveInstructions(MemLoadExp(node).address().explanation());
    case nkRelFeatherExpFieldRef:
        return canHaveInstructions(FieldRefExp(node).object().explanation());

    // These cannot have side effects
    case nkRelFeatherNop:
    case nkRelFeatherExpCtValue:
    case nkRelFeatherExpNull:
    case nkRelFeatherExpFunRef:
    case nkRelFeatherExpVarRef:
        return false;

    // For all other nodes, be conservative, and assume they have side effects
    default:
        return true;
    }
}

void writeHex(ostringstream& os, StringRef data) {
    os << "0x" << hex;
    for (const char* p = data.begin; p != data.end; p++) {
        unsigned char uc = *p;
        if (uc < 16)
            os << '0';
        os << (int)uc;
    }
    os << dec;
}

template <typename T> T extractValue(StringRef valueData) {
    ASSERT(valueData.size() == sizeof(T));
    // NOLINTNEXTLINE
    return *reinterpret_cast<const T*>(valueData.begin);
}

bool isField(VarDecl node) {
    // Check the direct parent is a class that contains the given node
    Nest_SymTab* st = node.context()->currentSymTab;
    NodeHandle parent = st ? st->node : nullptr;
    parent = parent ? parent.explanation() : NodeHandle{};
    if (parent && parent.kind() == nkFeatherDeclClass) {
        for (auto f : parent.children())
            if (f == node)
                return true;
    }
    return false;
}

bool _getBoolCtValue(Node* ctVal) { return (0 != *Feather_getCtValueData<unsigned char>(ctVal)); }

} // namespace

DEFINE_NODE_COMMON_IMPL(Nop, NodeHandle);
Nop Nop::create(const Location& loc) { return createNode<Nop>(loc); }
NodeHandle Nop::semanticCheckImpl(Nop node) {
    node.setType(VoidType::get(node.context()->evalMode));
    return node;
}

DEFINE_NODE_COMMON_IMPL(TypeNode, NodeHandle);
TypeNode TypeNode::create(const Location& loc, Type type) {
    TypeNode res = createNode<TypeNode>(loc);
    res.setProperty("givenType", type);
    return res;
}
Type TypeNode::givenType() const { return getCheckPropertyType("givenType"); }
NodeHandle TypeNode::semanticCheckImpl(TypeNode node) {
    node.setType(node.givenType());
    return node;
}
const char* TypeNode::toStringImpl(TypeNode node) {
    ostringstream os;
    os << "TypeNode(" << node.givenType() << ")";
    return dupString(os.str().c_str());
}

DEFINE_NODE_COMMON_IMPL(BackendCode, NodeHandle);
BackendCode BackendCode::create(const Location& loc, StringRef code, EvalMode mode) {
    auto res = createNode<BackendCode>(loc);
    res.setProperty(propCode, code);
    res.setProperty(propEvalMode, (int)mode);
    return res;
}
StringRef BackendCode::code() const { return getCheckPropertyString(propCode); }
EvalMode BackendCode::mode() const { return (EvalMode)getCheckPropertyInt(propEvalMode); }
NodeHandle BackendCode::semanticCheckImpl(BackendCode node) {
    if (!node.type())
        node.setType(VoidType::get(node.mode()));

    // CT process this node right after semantic check
    node.addModifier(&ctProcessMod);
    return node;
}
const char* BackendCode::toStringImpl(BackendCode node) {
    ostringstream os;
    os << "BackendCode(" << node.code().toStd() << ")";
    return dupString(os.str().c_str());
}

DEFINE_NODE_COMMON_IMPL(NodeList, NodeHandle);
NodeList NodeList::create(const Location& loc, NodeRange children, bool setVoid) {
    NodeList res = createNode<NodeList>(loc);
    res.setChildren(children);
    if (setVoid)
        res.setProperty(propResultVoid, 1);
    return res;
}
NodeList NodeList::append(NodeList prev, NodeHandle node) {
    NodeList res = prev;
    if (!res) {
        Location loc = node ? node.location() : Location{};
        res = create(loc, NodeRange{}, true);
    }
    res.addChild(node);
    return res;
}
NodeList NodeList::append(NodeList prev, NodeList newNodes) {
    if (!prev)
        return newNodes;
    if (!newNodes)
        return prev;

    prev.addChildren(newNodes.children());
    return prev;
}
bool NodeList::returnsVoid() const { return hasProperty(propResultVoid); }
Type NodeList::computeTypeImpl(NodeList node) {
    // If this returns void, make sure we set the right return type early on
    if (node.returnsVoid())
        node.setType(VoidType::get(node.context()->evalMode));

    // Compute the type for all the children
    for (auto child : node.children()) {
        if (!child)
            continue;
        child.computeType();
    }

    if (node.returnsVoid())
        return node.type();

    // Get the type of the last node
    int numChildren = node.children().size();
    Type res = (numChildren == 0 || !node.children()[numChildren - 1].type())
                       ? VoidType::get(node.context()->evalMode)
                       : node.children()[numChildren - 1].type();
    res = Feather_adjustMode(res, node.context(), node.location());
    return res;
}
NodeHandle NodeList::semanticCheckImpl(NodeList node) {
    // Semantic check each of the children
    bool hasNonCtChildren = false;
    for (auto child : node.children()) {
        if (!child)
            continue;
        if (!child.semanticCheck())
            return {};
        hasNonCtChildren = hasNonCtChildren || !Feather_isCt(child);
    }

    // Make sure the type is computed
    if (!node.type()) {
        // Get the type of the last node
        int numChildren = node.children().size();
        Type t =
                (node.returnsVoid() || numChildren == 0 || !node.children()[numChildren - 1].type())
                        ? VoidType::get(node.context()->evalMode)
                        : node.children()[numChildren - 1].type();
        t = Feather_adjustMode(t, node.context(), node.location());
        node.setType(t);
        Feather_checkEvalMode(node);
    }
    return node;
}

DEFINE_NODE_COMMON_IMPL(LocalSpace, NodeHandle);
LocalSpace LocalSpace::create(const Location& loc, NodeRange children) {
    LocalSpace res = createNode<LocalSpace>(loc);
    res.setChildren(children);
    return res;
}
void LocalSpace::setContextForChildrenImpl(LocalSpace node) {
    node.setChildrenContext(Nest_mkChildContextWithSymTab(node.context(), node, modeUnspecified));
    NodeHandle::setContextForChildrenImpl(node);
}
Type LocalSpace::computeTypeImpl(LocalSpace node) {
    return VoidType::get(node.context()->evalMode);
}
NodeHandle LocalSpace::semanticCheckImpl(LocalSpace node) {
    // Compute type first
    node.computeType();

    // Semantic check each of the children
    for (auto child : node.children()) {
        child.semanticCheck(); // Ignore possible errors
    }
    Feather_checkEvalMode(node);
    return node;
}

DEFINE_NODE_COMMON_IMPL(GlobalConstructAction, NodeHandle);
GlobalConstructAction GlobalConstructAction::create(const Location& loc, NodeHandle action) {
    GlobalConstructAction res = createNode<GlobalConstructAction>(loc);
    res.setChildren(NodeRange{action});
    return res;
}
NodeHandle GlobalConstructAction::action() const { return children()[0]; }
NodeHandle GlobalConstructAction::semanticCheckImpl(GlobalConstructAction node) {
    NodeHandle act = node.action();
    if (!act.semanticCheck())
        return {};

    node.setType(VoidType::get(node.context()->evalMode));

    // For CT construct actions, evaluate them asap
    if (Feather_isCt(act)) {
        Nest_ctEval(act);
        return Nop::create(node.location());
    }
    return node;
}

DEFINE_NODE_COMMON_IMPL(GlobalDestructAction, NodeHandle);
GlobalDestructAction GlobalDestructAction::create(const Location& loc, NodeHandle action) {
    GlobalDestructAction res = createNode<GlobalDestructAction>(loc);
    res.setChildren(NodeRange{action});
    return res;
}
NodeHandle GlobalDestructAction::action() const { return children()[0]; }
NodeHandle GlobalDestructAction::semanticCheckImpl(GlobalDestructAction node) {
    NodeHandle act = node.action();
    if (!act.semanticCheck())
        return {};

    node.setType(VoidType::get(node.context()->evalMode));

    // We never CT evaluate global destruct actions
    if (Feather_isCt(act)) {
        return Nop::create(node.location());
    }
    return node;
}

DEFINE_NODE_COMMON_IMPL(ScopeDestructAction, NodeHandle);
ScopeDestructAction ScopeDestructAction::create(const Location& loc, NodeHandle action) {
    ScopeDestructAction res = createNode<ScopeDestructAction>(loc);
    res.setChildren(NodeRange{action});
    return res;
}
NodeHandle ScopeDestructAction::action() const { return children()[0]; }
NodeHandle ScopeDestructAction::semanticCheckImpl(ScopeDestructAction node) {
    NodeHandle act = node.action();
    if (!act.semanticCheck())
        return {};

    node.setType(VoidType::get(node.context()->evalMode));
    return node;
}

DEFINE_NODE_COMMON_IMPL(TempDestructAction, NodeHandle);
TempDestructAction TempDestructAction::create(const Location& loc, NodeHandle action) {
    TempDestructAction res = createNode<TempDestructAction>(loc);
    res.setChildren(NodeRange{action});
    return res;
}
NodeHandle TempDestructAction::action() const { return children()[0]; }
NodeHandle TempDestructAction::semanticCheckImpl(TempDestructAction node) {
    NodeHandle act = node.action();
    if (!act.semanticCheck())
        return {};

    node.setType(VoidType::get(node.context()->evalMode));
    return node;
}

DEFINE_NODE_COMMON_IMPL(ChangeMode, NodeHandle);
ChangeMode ChangeMode::create(const Location& loc, NodeHandle child, EvalMode mode) {
    ChangeMode res = createNode<ChangeMode>(loc);
    res.setChildren(NodeRange{child});
    res.setProperty(propEvalMode, (int)mode);
    return res;
}
NodeHandle ChangeMode::child() const { return children()[0]; }
EvalMode ChangeMode::mode() const { return (EvalMode)getCheckPropertyInt(propEvalMode); }
void ChangeMode::setChild(NodeHandle child) {
    REQUIRE_NODE(NOLOC, child);
    childrenM()[0] = child;

    auto childrenCtx = childrenContext();
    if (childrenCtx)
        child.setContext(childrenCtx);
}
void ChangeMode::setContextForChildrenImpl(ChangeMode node) {
    auto curMode = node.mode();
    EvalMode newMode = curMode != modeUnspecified ? curMode : node.context()->evalMode;
    node.setChildrenContext(Nest_mkChildContext(node.context(), newMode));
    NodeHandle::setContextForChildrenImpl(node);
}
NodeHandle ChangeMode::semanticCheckImpl(ChangeMode node) {
    NodeHandle exp = node.child();

    // Make sure we are allowed to change the mode
    EvalMode baseMode = node.context()->evalMode;
    auto curMode = node.mode();
    EvalMode newMode = curMode != modeUnspecified ? curMode : baseMode;
    if (newMode == modeUnspecified)
        REP_INTERNAL(node.location(), "Cannot change the mode to Unspecified");
    if (newMode == modeRt && baseMode != modeRt)
        REP_ERROR_RET(nullptr, node.location(), "Cannot change mode to RT in a CT context (%1%)") %
                baseMode;

    if (!exp)
        REP_INTERNAL(node.location(), "No node specified as child to a ChangeMode node");

    return exp.semanticCheck();
}
const char* ChangeMode::toStringImpl(ChangeMode node) {
    ostringstream os;
    os << "changeMode(" << node.child() << ", " << node.mode() << ")";
    return dupString(os.str().c_str());
}

DeclNode::DeclNode(Node* n)
    : NodeHandle(n) {}
StringRef DeclNode::name() const { return getCheckPropertyString("name"); }
EvalMode DeclNode::mode() const { return (EvalMode)getCheckPropertyInt(propEvalMode); }
EvalMode DeclNode::effectiveMode() const {
    EvalMode nodeMode = mode();
    return nodeMode != modeUnspecified ? nodeMode : context()->evalMode;
}
void DeclNode::setMode(EvalMode mode) { setProperty(propEvalMode, (int)mode); }
void DeclNode::setNameAndMode(StringRef name, EvalMode mode) {
    setProperty("name", name);
    setProperty(propEvalMode, mode);
}

DEFINE_NODE_COMMON_IMPL(FunctionDecl, DeclNode);
FunctionDecl FunctionDecl::create(const Location& loc, StringRef name, NodeHandle resType,
        NodeRange params, NodeHandle body) {
    FunctionDecl res = createNode<FunctionDecl>(loc);
    res.setChildren(NodeRange{resType, body});
    res.addChildren(params);
    res.setNameAndMode(name, modeUnspecified);
    res.setProperty("callConvention", (int)ccC);

    return res;
}
NodeHandle FunctionDecl::resTypeNode() const { return children()[0]; }
NodeRange FunctionDecl::parameters() const { return children().skip(2); }
NodeHandle FunctionDecl::body() const { return children()[1]; }
CallConvention FunctionDecl::callConvention() const {
    return (CallConvention)getCheckPropertyInt("callConvention");
}
void FunctionDecl::addParameter(DeclNode param, bool insertInFront) {
    if (insertInFront)
        Nest_insertNodeIntoArray(&handle->children, 2, param);
    else
        Nest_appendNodeToArray(&handle->children, param);
}
void FunctionDecl::setResultType(NodeHandle resType) {
    childrenM()[0] = resType;
    resType.setContext(childrenContext());
}
void FunctionDecl::setBody(NodeHandle body) {
    childrenM()[1] = body;
    body.setContext(childrenContext());
}
void FunctionDecl::setContextForChildrenImpl(FunctionDecl node) {
    // If we don't have a children context, create one
    if (!node.hasDedicatedChildrenContext())
        node.setChildrenContext(Nest_mkChildContextWithSymTab(
                node.context(), node, Feather_effectiveEvalMode(node)));

    NodeHandle::setContextForChildrenImpl(node);

    Feather_addToSymTab(node);
}
Type FunctionDecl::computeTypeImpl(FunctionDecl node) {
    if (node.name().empty())
        REP_ERROR_RET(nullptr, node.location(), "No name given to function declaration");

    // We must have a result type
    Type resType = node.resTypeNode().computeType();
    if (!resType)
        REP_ERROR_RET(nullptr, node.location(), "No result type given to function %1%") %
                node.name();

    NodeRange params = node.parameters();

    vector<TypeRef> subTypes;
    subTypes.reserve(params.size() + 1);
    subTypes.push_back(resType);

    // Get the type for all the parameters
    for (auto param : params) {
        if (!param)
            REP_ERROR_RET(nullptr, node.location(), "Invalid parameter");
        if (!param.computeType())
            return {};
        if (param.explanation().kind() != nkFeatherDeclVar)
            REP_INTERNAL(param.location(), "Node %1% must be a parameter (%2%)") % param %
                    Nest_toStringEx(param);
        subTypes.push_back(param.type());
    }

    return FunctionType::get(&subTypes[0], subTypes.size(), Feather_effectiveEvalMode(node));
}
NodeHandle FunctionDecl::semanticCheckImpl(FunctionDecl node) {
    // Make sure the type is computed
    if (!node.computeType())
        return {};

    // Semantically check all the parameters
    for (auto param : node.parameters()) {
        if (!param.semanticCheck())
            return {};
    }

    // Semantically check the body, if we have one
    auto body = node.body();
    if (body)
        body.semanticCheck(); // Ignore possible errors

    // Do we have some valid instructions in the body?
    // Could we mark this function as having an empty body?
    auto resultType = node.resTypeNode();
    bool noRetValue = !resultType || !resultType.type().hasStorage();
    bool couldInline = !node.hasProperty(propNativeName) || !node.hasProperty(propNoInline);
    if (noRetValue && couldInline && body && !canHaveInstructions(body)) {
        node.setProperty(propEmptyBody, 1);
    }

    // TODO (function): Check that all the paths return a value
    return node;
}
const char* FunctionDecl::toStringImpl(FunctionDecl node) {
    ostringstream os;
    os << node.name();
    if (node.type()) {
        auto params = node.parameters();
        Type resultType = node.resTypeNode().type();
        if (node.hasProperty(propResultParam)) {
            resultType = params[0].type();
            resultType = removeCatOrRef(TypeWithStorage(resultType));
            params = params.skip(1); // Keep only the non-result params
        }

        os << '(';
        bool first = true;
        for (auto param : params) {
            if (first)
                first = false;
            else
                os << ", ";
            os << param.type();
        }
        os << "): " << resultType;
    }
    return dupString(os.str().c_str());
}

DEFINE_NODE_COMMON_IMPL(StructDecl, DeclNode);
StructDecl StructDecl::create(const Location& loc, StringRef name, NodeRange fields) {
    StructDecl res = createNode<StructDecl>(loc);
    res.setChildren(fields);
    res.setNameAndMode(name, modeUnspecified);

    // Make sure all the nodes given as fields have the right kind
    for (NodeHandle field : fields) {
        if (field.explanation().kind() != nkFeatherDeclVar)
            REP_INTERNAL(field.location(), "Node %1% must be a field") % field;
    }

    return res;
}
NodeRange StructDecl::fields() const { return children(); }
void StructDecl::setContextForChildrenImpl(StructDecl node) {
    // If we don't have a children context, create one
    if (!node.hasDedicatedChildrenContext())
        node.setChildrenContext(Nest_mkChildContextWithSymTab(
                node.context(), node, Feather_effectiveEvalMode(node)));

    NodeHandle::setContextForChildrenImpl(node);

    Feather_addToSymTab(node);
}
Type StructDecl::computeTypeImpl(StructDecl node) {
    if (node.name().empty())
        REP_ERROR_RET(nullptr, node.location(), "No name given to struct");

    // Compute the type for all the fields
    for (auto field : node.children()) {
        // Ignore errors from children
        field.computeType();
    }
    return DataType::get(node, 0, Feather_effectiveEvalMode(node));
}
NodeHandle StructDecl::semanticCheckImpl(StructDecl node) {
    if (!node.computeType())
        return {};

    // Semantically check all the fields
    for (auto field : node.children())
        field.semanticCheck(); // Ignore possible errors
    return node;
}

DEFINE_NODE_COMMON_IMPL(VarDecl, DeclNode);
VarDecl VarDecl::create(const Location& loc, StringRef name, NodeHandle typeNode) {
    VarDecl res = createNode<VarDecl>(loc);
    res.setChildren(NodeRange{typeNode});
    res.setNameAndMode(name, modeUnspecified);
    res.setProperty("alignment", 0);
    return res;
}
NodeHandle VarDecl::typeNode() const { return children()[0]; }
void VarDecl::setContextForChildrenImpl(VarDecl node) {
    NodeHandle::setContextForChildrenImpl(node);
    Feather_addToSymTab(node);
}
Type VarDecl::computeTypeImpl(VarDecl node) {
    // Make sure the variable has a type
    auto tn = node.typeNode();
    if (!tn.computeType())
        return {};

    // Adjust the mode of the type
    return Feather_adjustMode(tn.type(), node.context(), node.location());
}
NodeHandle VarDecl::semanticCheckImpl(VarDecl node) {
    if (!node.computeType())
        return {};

    // Make sure that the type has storage
    if (!node.type().hasStorage())
        REP_ERROR_RET(nullptr, node.location(), "Variable type has no storage (%1%)") % node.type();

    NodeHandle datatypeDecl = node.type().referredNode();
    if (!datatypeDecl)
        REP_ERROR_RET(nullptr, node.location(), "Invalid type for variable: %1%") % node.type();

    datatypeDecl.computeType(); // Make sure the type of the decl is computed
    return node;
}

DEFINE_NODE_COMMON_IMPL(CtValueExp, NodeHandle);
CtValueExp CtValueExp::create(const Location& loc, TypeWithStorage type, StringRef data) {
    CtValueExp res = createNode<CtValueExp>(loc);
    res.setProperty("valueType", type);
    res.setProperty("valueData", data);
    return res;
}
TypeWithStorage CtValueExp::valueType() const {
    return TypeWithStorage(getCheckPropertyType("valueType"));
}
StringRef CtValueExp::valueData() const { return getCheckPropertyString("valueData"); }
NodeHandle CtValueExp::semanticCheckImpl(CtValueExp node) {
    // Check the type
    if (!node.type())
        node.setType(node.valueType());
    if (!node.type() || !node.type().hasStorage())
        REP_ERROR_RET(nullptr, node.location(),
                "Type specified for Ct Value cannot be used at compile-time (%1%)") %
                node.type();

    // Make sure data size matches the size reported by the type
    unsigned valueSize = Nest_sizeOf(node.type());
    StringRef data = node.valueData();
    if (valueSize != data.size()) {
        REP_ERROR_RET(nullptr, node.location(),
                "Read value size (%1%) differs from declared size of the value (%2%) - type: %3%") %
                data.size() % valueSize % node.type();
    }

    node.setType(node.type().changeMode(modeCt, node.location()));
    return node;
}
const char* CtValueExp::toStringImpl(ThisNodeType node) {
    Type t = node.type();
    if (!t)
        t = node.valueType();
    if (!t)
        return "CtValue";
    ostringstream os;
    os << "CtValue(" << t << ": ";

    StringRef valueDataStr = node.valueData();

    StringRef nativeName = t.hasStorage() ? Feather_nativeName(t) : StringRef{};
    if (0 == strcmp(t.description(), "Type/ct")) {
        auto t = extractValue<Type>(valueDataStr);
        os << t;
    } else if (nativeName && numRefs(t) == 0) {
        if (nativeName == "i1" || nativeName == "u1") {
            bool val = 0 != extractValue<uint8_t>(valueDataStr);
            os << (val ? "true" : "false");
        } else if (nativeName == "i16")
            os << extractValue<int16_t>(valueDataStr);
        else if (nativeName == "u16")
            os << extractValue<uint16_t>(valueDataStr);
        else if (nativeName == "i32")
            os << extractValue<int32_t>(valueDataStr);
        else if (nativeName == "u32")
            os << extractValue<uint32_t>(valueDataStr);
        else if (nativeName == "i64")
            os << extractValue<int64_t>(valueDataStr);
        else if (nativeName == "u64")
            os << extractValue<uint64_t>(valueDataStr);
        else if (nativeName == "float")
            os << extractValue<float>(valueDataStr);
        else if (nativeName == "double")
            os << extractValue<double>(valueDataStr);
        else if (nativeName == "StringRef") {
            os << "'" << extractValue<StringRef>(valueDataStr) << "'";
        } else
            writeHex(os, valueDataStr);
    } else
        writeHex(os, valueDataStr);
    os << ")";
    return dupString(os.str().c_str());
}

DEFINE_NODE_COMMON_IMPL(NullExp, NodeHandle);
NullExp NullExp::create(const Location& loc, NodeHandle typeNode) {
    NullExp res = createNode<NullExp>(loc);
    res.setChildren(NodeRange{typeNode});
    return res;
}
NodeHandle NullExp::typeNode() const { return children()[0]; }
NodeHandle NullExp::semanticCheckImpl(NullExp node) {
    Type t = node.typeNode().computeType();
    if (!t)
        return {};

    // Make sure that the type is a reference
    if (numRefs(t) < 1)
        REP_ERROR_RET(nullptr, node.location(),
                "Null node should have a reference type (cur type: %1%)") %
                t;

    node.setType(Feather_adjustMode(t, node.context(), node.location()));
    return node;
}
const char* NullExp::toStringImpl(NullExp node) {
    ostringstream os;
    os << "Null(" << node.type() << ")";
    return dupString(os.str().c_str());
}

DEFINE_NODE_COMMON_IMPL(VarRefExp, NodeHandle);
VarRefExp VarRefExp::create(const Location& loc, VarDecl varDecl) {
    REQUIRE_NODE(loc, varDecl);

    VarRefExp res = createNode<VarRefExp>(loc);
    res.setReferredNodes(NodeRange{varDecl});
    return res;
}
VarDecl VarRefExp::varDecl() const { return VarDecl(referredNodes()[0]); }
NodeHandle VarRefExp::semanticCheckImpl(VarRefExp node) {
    VarDecl var = node.varDecl();
    ASSERT(var);
    if (!var.computeType())
        return {};
    if (isField(var))
        REP_INTERNAL(node.location(), "VarRef used on a field (%1%). Use FieldRef instead") %
                var.name();
    if (!var.type().hasStorage())
        REP_ERROR_RET(
                nullptr, node.location(), "Variable type doesn't have a storage type (type: %1%)") %
                var.type();
    TypeWithStorage t = var.type();
    if (!Feather::isCategoryType(t))
        t = MutableType::get(t);
    node.setType(Feather_adjustMode(t, node.context(), node.location()));
    Feather_checkEvalModeWithExpected(node, var.type().mode());
    return node;
}
const char* VarRefExp::toStringImpl(VarRefExp node) {
    ostringstream os;
    os << "VarRef(" << node.varDecl().name() << ")";
    return dupString(os.str().c_str());
}

DEFINE_NODE_COMMON_IMPL(FieldRefExp, NodeHandle);
FieldRefExp FieldRefExp::create(const Location& loc, NodeHandle obj, VarDecl fieldDecl) {
    REQUIRE_NODE(loc, obj);
    REQUIRE_NODE(loc, fieldDecl);
    FieldRefExp res = createNode<FieldRefExp>(loc);
    res.setChildren(NodeRange{obj});
    res.setReferredNodes(NodeRange{fieldDecl});
    return res;
}
NodeHandle FieldRefExp::object() const { return children()[0]; }
VarDecl FieldRefExp::fieldDecl() const { return VarDecl(referredNodes()[0]); }
NodeHandle FieldRefExp::semanticCheckImpl(FieldRefExp node) {
    NodeHandle obj = node.object();
    VarDecl field = node.fieldDecl();
    ASSERT(obj);
    ASSERT(field);

    // Semantic check the object node - make sure it's a reference to a data type
    if (!obj.semanticCheck())
        return {};
    ASSERT(obj.type());
    if (numRefs(obj.type()) != 1)
        REP_ERROR_RET(nullptr, node.location(),
                "Field access should be done on a reference to a data type (type: %1%)") %
                obj.type();
    NodeHandle structNode = TypeWithStorage(obj.type()).referredNode();
    ASSERT(structNode);
    if (!structNode.computeType())
        return {};

    // Compute the type of the field
    if (!field.computeType())
        return {};

    // Make sure that the type of a object is a data type that refers to a class the contains the
    // given field
    bool fieldFound = false;
    for (auto f : structNode.children()) {
        if (field == f) {
            fieldFound = true;
            break;
        }
    }
    if (!fieldFound)
        REP_ERROR_RET(nullptr, node.location(), "Field '%1%' not found when accessing object") %
                field.name();

    // Set the correct type for this node
    TypeWithStorage t = field.type();
    ASSERT(t);
    if (!Feather::isCategoryType(t))
        t = MutableType::get(t);
    node.setType(t);
    EvalMode mode = Feather_combineMode(obj.type().mode(), node.context()->evalMode);
    node.setType(node.type().changeMode(mode, node.location()));
    return node;
}
const char* FieldRefExp::toStringImpl(FieldRefExp node) {
    ostringstream os;
    os << "FieldRef(" << node.fieldDecl().name() << ")";
    return dupString(os.str().c_str());
}

DEFINE_NODE_COMMON_IMPL(FunRefExp, NodeHandle);
FunRefExp FunRefExp::create(const Location& loc, FunctionDecl funDecl, NodeHandle resType) {
    REQUIRE_NODE(loc, funDecl);
    REQUIRE_NODE(loc, resType);
    FunRefExp res = createNode<FunRefExp>(loc);
    res.setChildren(NodeRange{resType});
    res.setReferredNodes(NodeRange{funDecl});
    return res;
}
FunctionDecl FunRefExp::funDecl() const { return FunctionDecl(referredNodes()[0]); }
NodeHandle FunRefExp::resTypeNode() const { return children()[0]; }
NodeHandle FunRefExp::semanticCheckImpl(FunRefExp node) {
    NodeHandle resType = node.resTypeNode();
    FunctionDecl fun = node.funDecl();
    if (!resType.computeType())
        return {};

    if (!fun.computeType())
        return {};
    node.setType(Feather_adjustMode(resType.type(), node.context(), node.location()));
    return node;
}
const char* FunRefExp::toStringImpl(FunRefExp node) {
    ostringstream os;
    os << "FunRef(" << node.funDecl() << ")";
    return dupString(os.str().c_str());
}

DEFINE_NODE_COMMON_IMPL(FunCallExp, NodeHandle);
FunCallExp FunCallExp::create(const Location& loc, FunctionDecl funDecl, NodeRange args) {
    REQUIRE_NODE(loc, funDecl);
    FunCallExp res = createNode<FunCallExp>(loc);
    res.setChildren(args);
    res.setReferredNodes(NodeRange{funDecl});
    return res;
}
FunctionDecl FunCallExp::funDecl() const { return FunctionDecl(referredNodes()[0]); }
NodeRange FunCallExp::arguments() const { return children(); }
NodeHandle FunCallExp::semanticCheckImpl(FunCallExp node) {
    FunctionDecl fun = node.funDecl();

    // Make sure the function declaration is has a valid type
    if (!fun.computeType())
        return {};

    // Check argument count
    int numParameters = fun.parameters().size();
    int numArgs = node.arguments().size();
    if (numArgs != numParameters)
        REP_ERROR_RET(nullptr, node.location(),
                "Invalid function call: expecting %1% parameters, given %2%") %
                numParameters % numArgs;

    // Semantic check the arguments
    // Also check that their type matches the corresponding type from the function decl
    bool allParamsAreCtAvailable = true;
    for (int i = 0; i < numArgs; ++i) {
        NodeHandle arg = node.arguments()[i];

        // Semantically check the argument
        if (!arg.semanticCheck())
            return {};
        if (!Feather_isCt(arg))
            allParamsAreCtAvailable = false;

        // Compare types
        Type argType = arg.type();
        Type paramType = fun.parameters()[i].type();
        if (!sameTypeIgnoreMode(argType, paramType))
            REP_ERROR_RET(nullptr, arg.location(),
                    "Invalid function call; parameter %1% expected type %2%, given %3%") %
                    fun.parameters()[i] % paramType % argType;
    }

    // CT availability checks
    EvalMode curMode = node.context()->evalMode;
    EvalMode calledFunMode = Feather_effectiveEvalMode(fun);
    ASSERT(curMode != modeUnspecified);
    ASSERT(calledFunMode != modeUnspecified);
    if (calledFunMode == modeCt && curMode != modeCt && !allParamsAreCtAvailable) {
        REP_ERROR(node.location(),
                "Not all arguments are compile-time, when calling a compile time function");
        REP_INFO(fun.location(), "See called function");
        return {};
    }

    // Get the type from the function decl
    node.setType(fun.resTypeNode().type());

    // Handle autoCt case
    if (allParamsAreCtAvailable && node.type().mode() == modeRt && fun.hasProperty(propAutoCt)) {
        node.setType(node.type().changeMode(modeCt, node.location()));
    }

    // Make sure we yield a type with the right mode
    node.setType(Feather_adjustMode(node.type(), node.context(), node.location()));

    // Check if this node can have some meaningful instructions generated for it
    bool noRetValue = !node.type().hasStorage();
    if (noRetValue && !canHaveInstructions(node)) {
        node.setProperty(propEmptyBody, 1);
    }

    Feather_checkEvalModeWithExpected(node, calledFunMode);
    return node;
}
const char* FunCallExp::toStringImpl(FunCallExp node) {
    ostringstream os;
    os << "FunCall-" << node.funDecl().name() << "(" << node.arguments() << ")";
    return dupString(os.str().c_str());
}

DEFINE_NODE_COMMON_IMPL(MemLoadExp, NodeHandle);
MemLoadExp MemLoadExp::create(const Location& loc, NodeHandle address) {
    REQUIRE_NODE(loc, address);
    MemLoadExp res = createNode<MemLoadExp>(loc);
    res.setChildren(NodeRange{address});
    res.setProperty("alignment", 0);
    res.setProperty("volatile", 0);
    res.setProperty("atomicOrdering", (int)atomicNone);
    res.setProperty("singleThreaded", 0);
    return res;
}
NodeHandle MemLoadExp::address() const { return children()[0]; }
NodeHandle MemLoadExp::semanticCheckImpl(MemLoadExp node) {
    NodeHandle exp = node.address();

    // Semantic check the argument
    if (!exp.semanticCheck())
        return {};

    // Check if the type of the argument is a ref
    if (numRefs(exp.type()) < 1)
        REP_ERROR_RET(
                nullptr, node.location(), "Cannot load from a non-reference (%1%, type: %2%)") %
                exp % exp.type();

    // Check flags
    auto ordering = (AtomicOrdering)node.getCheckPropertyInt("atomicOrdering");
    if (ordering == atomicRelease)
        REP_ERROR_RET(
                nullptr, node.location(), "Cannot use atomic release with a load instruction");
    if (ordering == atomicAcquireRelease)
        REP_ERROR_RET(nullptr, node.location(),
                "Cannot use atomic acquire-release with a load instruction");

    // Remove the 'ref' from the type and get the base type
    node.setType(removeCatOrRef(TypeWithStorage(exp.type())));
    node.setType(Feather_adjustMode(node.type(), node.context(), node.location()));
    return node;
}

DEFINE_NODE_COMMON_IMPL(MemStoreExp, NodeHandle);
MemStoreExp MemStoreExp::create(const Location& loc, NodeHandle value, NodeHandle address) {
    REQUIRE_NODE(loc, value);
    REQUIRE_NODE(loc, address);
    MemStoreExp res = createNode<MemStoreExp>(loc);
    res.setChildren(NodeRange{value, address});
    res.setProperty("alignment", 0);
    res.setProperty("volatile", 0);
    res.setProperty("atomicOrdering", (int)atomicNone);
    res.setProperty("singleThreaded", 0);
    return res;
}
NodeHandle MemStoreExp::value() const { return children()[0]; }
NodeHandle MemStoreExp::address() const { return children()[1]; }
NodeHandle MemStoreExp::semanticCheckImpl(MemStoreExp node) {
    NodeHandle value = node.value();
    NodeHandle address = node.address();
    ASSERT(value);
    ASSERT(address);

    // Semantic check the arguments
    if (!value.semanticCheck() || !address.semanticCheck())
        return {};

    // Check if the type of the address is a ref
    if (numRefs(address.type()) < 1)
        REP_ERROR_RET(nullptr, node.location(),
                "The address of a memory store is not a reference, nor VarRef nor FieldRef (type: "
                "%1%)") %
                address.type();
    Type baseAddressType = removeCatOrRef(TypeWithStorage(address.type()));

    // Check the equivalence of types
    if (!sameTypeIgnoreMode(value.type(), baseAddressType)) {
        // Try again, getting rid of category type
        if (!sameTypeIgnoreMode(categoryToRefIfPresent(value.type()), baseAddressType))
            REP_ERROR_RET(nullptr, node.location(),
                    "The type of the value doesn't match the type of the address in a memory store "
                    "(%1% != %2%)") %
                    value.type() % baseAddressType;
    }

    // The resulting type is Void
    node.setType(VoidType::get(address.type().mode()));
    return node;
}

DEFINE_NODE_COMMON_IMPL(BitcastExp, NodeHandle);
BitcastExp BitcastExp::create(const Location& loc, NodeHandle destType, NodeHandle exp) {
    REQUIRE_NODE(loc, destType);
    REQUIRE_NODE(loc, exp);
    BitcastExp res = createNode<BitcastExp>(loc);
    res.setChildren(NodeRange{exp, destType});
    return res;
}
NodeHandle BitcastExp::destTypeNode() const { return children()[1]; }
NodeHandle BitcastExp::expression() const { return children()[0]; }
NodeHandle BitcastExp::semanticCheckImpl(BitcastExp node) {
    if (!node.expression().semanticCheck())
        return {};
    Type tDest = node.destTypeNode().computeType();
    if (!tDest)
        return {};
    Type srcType = node.expression().type();

    // Make sure both types are references
    if (numRefs(srcType) < 1)
        REP_ERROR_RET(
                nullptr, node.location(), "The source of a bitcast is not a reference (%1%)") %
                srcType;
    if (numRefs(tDest) < 1)
        REP_ERROR_RET(nullptr, node.location(),
                "The destination type of a bitcast is not a reference (%1%)") %
                tDest;

    node.setType(Feather_adjustMode(tDest, node.context(), node.location()));
    return node;
}
const char* BitcastExp::toStringImpl(BitcastExp node) {
    ostringstream os;
    if (node.destTypeNode().type())
        os << "Bitcast(" << node.destTypeNode().type() << ", " << node.expression() << ")";
    else
        os << "Bitcast(type(" << node.destTypeNode() << "), " << node.expression() << ")";
    return dupString(os.str().c_str());
}

DEFINE_NODE_COMMON_IMPL(ConditionalExp, NodeHandle);
ConditionalExp ConditionalExp::create(
        const Location& loc, NodeHandle cond, NodeHandle alt1, NodeHandle alt2) {
    REQUIRE_NODE(loc, cond);
    REQUIRE_NODE(loc, alt1);
    REQUIRE_NODE(loc, alt2);
    ConditionalExp res = createNode<ConditionalExp>(loc);
    res.setChildren(NodeRange{cond, alt1, alt2});
    return res;
}
NodeHandle ConditionalExp::condition() const { return children()[0]; }
NodeHandle ConditionalExp::alt1() const { return children()[1]; }
NodeHandle ConditionalExp::alt2() const { return children()[2]; }
NodeHandle ConditionalExp::semanticCheckImpl(ConditionalExp node) {
    NodeHandle& cond = node.childrenM()[0];
    NodeHandle alt1 = node.alt1();
    NodeHandle alt2 = node.alt2();

    // Semantic check the condition
    if (!cond.semanticCheck())
        return {};

    // Check that the type of the condition is 'Testable'
    if (!Feather_isTestable(cond))
        REP_ERROR_RET(nullptr, cond.location(),
                "The condition of the conditional expression is not Testable (%1%)") %
                cond.type();

    // Dereference the condition as much as possible
    while (cond.type() && numRefs(cond.type()) > 0) {
        cond = MemLoadExp::create(cond.location(), cond);
        cond.setContext(node.childrenContext());
        if (!cond.semanticCheck())
            return {};
    }
    ASSERT(cond.type());
    // TODO (conditional): This shouldn't be performed here

    // Semantic check the alternatives
    if (!alt1.semanticCheck() || !alt2.semanticCheck())
        return {};

    // Make sure the types of the alternatives are equal
    if (!sameTypeIgnoreMode(alt1.type(), alt2.type()))
        REP_ERROR_RET(nullptr, node.location(),
                "The types of the alternatives of a conditional must be equal (%1% != %2%)") %
                alt1.type() % alt2.type();

    EvalMode mode = Feather_combineModeBottom(alt1.type().mode(), cond.type().mode());
    mode = Feather_combineMode(mode, node.context()->evalMode);
    node.setType(Type(alt1.type()).changeMode(mode, node.location()));
    return node;
}

DEFINE_NODE_COMMON_IMPL(IfStmt, NodeHandle);
IfStmt IfStmt::create(const Location& loc, NodeHandle cond, NodeHandle thenC, NodeHandle elseC) {
    REQUIRE_NODE(loc, cond);
    IfStmt res = createNode<IfStmt>(loc);
    res.setChildren(NodeRange{cond, thenC, elseC});
    return res;
}
NodeHandle IfStmt::condition() const { return children()[0]; }
NodeHandle IfStmt::thenClause() const { return children()[1]; }
NodeHandle IfStmt::elseClause() const { return children()[2]; }
void IfStmt::setContextForChildrenImpl(IfStmt node) {
    node.setChildrenContext(Nest_mkChildContextWithSymTab(node.context(), node, modeUnspecified));
    NodeHandle::setContextForChildrenImpl(node);
}
NodeHandle IfStmt::semanticCheckImpl(IfStmt node) {
    NodeHandle condition = node.condition();
    NodeHandle thenClause = node.thenClause();
    NodeHandle elseClause = node.elseClause();

    // The resulting type is Void
    node.setType(VoidType::get(node.context()->evalMode));

    // Semantic check the condition
    if (!condition.semanticCheck())
        return {};

    // Check that the type of the condition is 'Testable'
    if (!Feather_isTestable(condition))
        REP_ERROR_RET(nullptr, condition.location(),
                "The condition of the if is not Testable; type=%1%") %
                condition.type();

    // Dereference the condition as much as possible
    while (condition.type() && numRefs(condition.type()) > 0) {
        condition = MemLoadExp::create(condition.location(), condition);
        condition.setContext(node.childrenContext());
        if (!condition.semanticCheck())
            return {};
    }
    node.childrenM()[0] = condition;
    // TODO (if): Remove this dereference from here

    if (Feather_nodeEvalMode(node) == modeCt) {
        if (!Feather_isCt(condition))
            REP_ERROR_RET(nullptr, condition.location(),
                    "The condition of the ct if should be available at compile-time (%1%)") %
                    condition.type();

        // Get the CT value from the condition, and select an active branch
        NodeHandle c = Nest_ctEval(condition);
        NodeHandle selectedBranch = _getBoolCtValue(c) ? thenClause : elseClause;

        // Expand only the selected branch
        if (selectedBranch)
            return selectedBranch;
        else
            return Nop::create(node.location());
    }

    // Semantic check the clauses
    if (thenClause && !thenClause.semanticCheck())
        return {};
    if (elseClause && !elseClause.semanticCheck())
        return {};
    return node;
}

DEFINE_NODE_COMMON_IMPL(WhileStmt, NodeHandle);
WhileStmt WhileStmt::create(
        const Location& loc, NodeHandle cond, NodeHandle body, NodeHandle step) {
    REQUIRE_NODE(loc, cond);
    REQUIRE_NODE(loc, body);
    WhileStmt res = createNode<WhileStmt>(loc);
    res.setChildren(NodeRange{cond, step, body});
    return res;
}
NodeHandle WhileStmt::condition() const { return children()[0]; }
NodeHandle WhileStmt::body() const { return children()[2]; }
NodeHandle WhileStmt::step() const { return children()[1]; }
void WhileStmt::setContextForChildrenImpl(WhileStmt node) {
    node.setChildrenContext(Nest_mkChildContextWithSymTab(node.context(), node, modeUnspecified));
    CompilationContext* condContext = Feather_nodeEvalMode(node) == modeCt
                                              ? Nest_mkChildContext(node.context(), modeCt)
                                              : node.childrenContext();

    node.condition().setContext(condContext);
    if (node.step())
        node.step().setContext(condContext);
    if (node.body())
        node.body().setContext(node.childrenContext());
}
NodeHandle WhileStmt::semanticCheckImpl(WhileStmt node) {
    NodeHandle condition = node.condition();
    NodeHandle step = node.step();
    NodeHandle body = node.body();

    // Semantic check the condition
    if (!condition.semanticCheck())
        return {};
    if (step && !step.computeType())
        return {};

    // Check that the type of the condition is 'Testable'
    if (!Feather_isTestable(condition))
        REP_ERROR_RET(nullptr, condition.location(), "The condition of the while is not Testable");

    // Dereference the condition as much as possible
    while (condition.type() && numRefs(condition.type()) > 0) {
        condition = MemLoadExp::create(condition.location(), condition);
        condition.setContext(node.childrenContext());
        if (!condition.semanticCheck())
            return {};
    }
    node.childrenM()[0] = condition;
    // TODO (while): Remove this dereference from here

    if (Feather_nodeEvalMode(node) == modeCt) {
        if (!Feather_isCt(condition))
            REP_ERROR_RET(nullptr, condition.location(),
                    "The condition of the ct while should be available at compile-time (%1%)") %
                    condition.type();
        if (step && !Feather_isCt(step))
            REP_ERROR_RET(nullptr, step.location(),
                    "The step of the ct while should be available at compile-time (%1%)") %
                    step.type();

        // Create a node-list that will be our explanation
        auto result = Nest_allocNodeArray(0);

        // Do the while
        while (true) {
            // CT-evaluate the condition; if the condition evaluates to false, exit the while
            if (!_getBoolCtValue(Nest_ctEval(condition)))
                break;

            // Put (a copy of) the body in the resulting node-list
            if (body) {
                NodeHandle curBody = body.clone();
                curBody.setContext(node.context());
                if (!curBody.semanticCheck())
                    return {};
                Nest_appendNodeToArray(&result, curBody);
            }

            // If we have a step, make sure to evaluate it
            if (step) {
                Nest_ctEval(step); // We don't need the actual result
            }

            // Unfortunately, we don't treat 'break' and 'continue' instructions inside the ct while
            // instructions
        }

        // Set the explanation and exit
        NodeHandle res = NodeList::create(node.location(), all(result), true);
        Nest_freeNodeArray(result);
        return res;
    }

    // Semantic check the body and the step
    if (body && !body.semanticCheck())
        return {};
    if (step && !step.semanticCheck())
        return {};

    // The resulting type is Void
    node.setType(VoidType::get(node.context()->evalMode));
    return node;
}

DEFINE_NODE_COMMON_IMPL(BreakStmt, NodeHandle);
BreakStmt BreakStmt::create(const Location& loc) {
    BreakStmt res = createNode<BreakStmt>(loc);
    res.setProperty("loop", NodeHandle{});
    return res;
}
NodeHandle BreakStmt::semanticCheckImpl(BreakStmt node) {
    // Get the outer-most loop from the context
    NodeHandle loop = Feather_getParentLoop(node.context());
    if (!loop)
        REP_ERROR_RET(nullptr, node.location(), "Break found outside any loop");
    node.setProperty("loop", loop);

    // The resulting type is Void
    node.setType(VoidType::get(node.context()->evalMode));
    return node;
}

DEFINE_NODE_COMMON_IMPL(ContinueStmt, NodeHandle);
ContinueStmt ContinueStmt::create(const Location& loc) {
    ContinueStmt res = createNode<ContinueStmt>(loc);
    res.setProperty("loop", NodeHandle{});
    return res;
}
NodeHandle ContinueStmt::semanticCheckImpl(ContinueStmt node) {
    // Get the outer-most loop from the context
    NodeHandle loop = Feather_getParentLoop(node.context());
    if (!loop)
        REP_ERROR_RET(nullptr, node.location(), "Continue found outside any loop");
    node.setProperty("loop", loop);

    // The resulting type is Void
    node.setType(VoidType::get(node.context()->evalMode));
    return node;
}

DEFINE_NODE_COMMON_IMPL(ReturnStmt, NodeHandle);
ReturnStmt ReturnStmt::create(const Location& loc, NodeHandle exp) {
    ReturnStmt res = createNode<ReturnStmt>(loc);
    res.setChildren(NodeRange{exp});
    res.setProperty("parentFun", NodeHandle{});
    return res;
}
NodeHandle ReturnStmt::expression() const { return children()[0]; }
NodeHandle ReturnStmt::semanticCheckImpl(ReturnStmt node) {
    NodeHandle exp = node.expression();

    // If we have an expression argument, semantically check it
    if (exp && !exp.semanticCheck())
        return {};

    // Get the parent function of this return
    NodeHandle parentFun = Feather_getParentFun(node.context());
    if (!parentFun)
        REP_ERROR_RET(nullptr, node.location(), "Return found outside any function");
    Type resultType = FunctionDecl(parentFun).resTypeNode().type();
    ASSERT(resultType);
    node.setProperty("parentFun", parentFun);

    // If the return has an expression, check that has the same type as the function result type
    if (exp) {
        if (!sameTypeIgnoreMode(exp.type(), resultType))
            REP_ERROR_RET(nullptr, node.location(),
                    "Returned expression's type is not the same as function's return type");
    } else {
        // Make sure that the function has a void return type
        if (resultType.kind() != typeKindVoid)
            REP_ERROR_RET(nullptr, node.location(),
                    "You must return something in a function that has non-Void result type");
    }

    // The resulting type is Void
    node.setType(VoidType::get(node.context()->evalMode));
    return node;
}

} // namespace Feather
