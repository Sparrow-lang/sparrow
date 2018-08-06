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

#define REQUIRE_NODE(loc, node)                                                                    \
    if (node)                                                                                      \
        ;                                                                                          \
    else                                                                                           \
        REP_INTERNAL((loc), "Expected AST node (%1%)") % (#node)

#define REQUIRE_NODE_KIND(node, kind)                                                              \
    if (node && node->nodeKind != kind)                                                            \
        REP_INTERNAL(NOLOC, "Expected AST node of kind %1%, found %2% (inside %3%)") % (#kind) %   \
                NodeHandle(node).kindName() % __FUNCTION__;                                        \
    else                                                                                           \
        ;

namespace Feather {

namespace {

const char* propResultVoid = "nodeList.resultVoid";

//! Helper function used to create nodes
template <typename T> T createNode(int kind, const Location& loc) {
    T res;
    res.handle = NodeHandle::create(kind, loc).handle;
    return res;
}

#define REGISTER_NODE_KIND_IMPL(T)                                                                 \
    int T::registerNodeKind() {                                                                    \
        struct RegHelper {                                                                         \
            static int registerKind(const char* name) {                                            \
                auto semCheck = reinterpret_cast<FSemanticCheck>(&semanticCheckPlain);             \
                auto compT = reinterpret_cast<FComputeType>(&computeTypePlain);                    \
                auto setCtx =                                                                      \
                        reinterpret_cast<FSetContextForChildren>(&setContextForChildrenPlain);     \
                auto toStr = reinterpret_cast<FToString>(&toStringPlain);                          \
                return Nest_registerNodeKind(name, semCheck, compT, setCtx, toStr);                \
            }                                                                                      \
                                                                                                   \
        private:                                                                                   \
            static NodeHandle semanticCheckPlain(T node) { return node.semanticCheckImpl(); }      \
            static TypeRef computeTypePlain(T node) { return node.computeTypeImpl(); }             \
            static void setContextForChildrenPlain(T node) { node.setContextForChildrenImpl(); }   \
            static const char* toStringPlain(T node) { return node.toStringImpl(); }               \
        };                                                                                         \
        return RegHelper::registerKind(#T);                                                        \
    }

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
    ASSERT(size(valueData) == sizeof(T));
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

REGISTER_NODE_KIND_IMPL(Nop);
Nop Nop::create(const Location& loc) { return createNode<Nop>(nkFeatherNop, loc); }
Nop::Nop(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherNop);
}
NodeHandle Nop::semanticCheckImpl() {
    handle->type = VoidType::get(context()->evalMode);
    return *this;
}

REGISTER_NODE_KIND_IMPL(TypeNode);
TypeNode TypeNode::create(const Location& loc, TypeBase type) {
    TypeNode res = createNode<TypeNode>(nkFeatherTypeNode, loc);
    res.setProperty("givenType", type);
    return res;
}
TypeNode::TypeNode(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherTypeNode);
}
TypeBase TypeNode::givenType() const { return getCheckPropertyType("givenType"); }
NodeHandle TypeNode::semanticCheckImpl() {
    handle->type = givenType();
    return *this;
}
const char* TypeNode::toStringImpl() {
    ostringstream os;
    os << "TypeNode(" << givenType() << ")";
    return dupString(os.str().c_str());
}

REGISTER_NODE_KIND_IMPL(BackendCode);
BackendCode BackendCode::create(const Location& loc, StringRef code, EvalMode mode) {
    auto res = createNode<BackendCode>(nkFeatherBackendCode, loc);
    res.setProperty(propCode, code);
    res.setProperty(propEvalMode, (int)mode);
    return res;
}
BackendCode::BackendCode(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherBackendCode);
}
StringRef BackendCode::code() const { return getCheckPropertyString(propCode); }
EvalMode BackendCode::mode() const { return (EvalMode)getCheckPropertyInt(propEvalMode); }
NodeHandle BackendCode::semanticCheckImpl() {
    if (!type())
        handle->type = VoidType::get(mode());

    // CT process this node right after semantic check
    addModifier(&ctProcessMod);
    return *this;
}
const char* BackendCode::toStringImpl() {
    ostringstream os;
    os << "BackendCode(" << code().toStd() << ")";
    return dupString(os.str().c_str());
}

REGISTER_NODE_KIND_IMPL(NodeList);
NodeList NodeList::create(const Location& loc, NodeRange children, bool setVoid) {
    NodeList res = createNode<NodeList>(nkFeatherNodeList, loc);
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
NodeList::NodeList(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherNodeList);
}
bool NodeList::returnsVoid() const { return hasProperty(propResultVoid); }
TypeRef NodeList::computeTypeImpl() {
    // If this returns void, make sure we set the right return type early on
    if (returnsVoid())
        handle->type = VoidType::get(context()->evalMode);

    // Compute the type for all the children
    for (auto child : children()) {
        if (!child)
            continue;
        child.computeType();
    }

    if (returnsVoid())
        return handle->type;

    // Get the type of the last node
    int numChildren = children().size();
    TypeRef res = (numChildren == 0 || !children()[numChildren - 1].type())
                          ? VoidType::get(context()->evalMode)
                          : children()[numChildren - 1].type();
    res = Feather_adjustMode(res, context(), location());
    return res;
}
NodeHandle NodeList::semanticCheckImpl() {
    // Semantic check each of the children
    bool hasNonCtChildren = false;
    for (auto child : children()) {
        if (!child)
            continue;
        if (!child.semanticCheck())
            return {};
        hasNonCtChildren = hasNonCtChildren || !Feather_isCt(child);
    }

    // Make sure the type is computed
    if (!type()) {
        // Get the type of the last node
        int numChildren = children().size();
        TypeRef t = (returnsVoid() || numChildren == 0 || !children()[numChildren - 1].type())
                            ? VoidType::get(context()->evalMode)
                            : children()[numChildren - 1].type();
        t = Feather_adjustMode(t, context(), location());
        handle->type = t;
        Feather_checkEvalMode(handle);
    }
    return *this;
}

REGISTER_NODE_KIND_IMPL(LocalSpace);
LocalSpace LocalSpace::create(const Location& loc, NodeRange children) {
    LocalSpace res = createNode<LocalSpace>(nkFeatherLocalSpace, loc);
    res.setChildren(children);
    return res;
}
LocalSpace::LocalSpace(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherLocalSpace);
}
void LocalSpace::setContextForChildrenImpl() {
    handle->childrenContext = Nest_mkChildContextWithSymTab(context(), handle, modeUnspecified);
    NodeHandle::setContextForChildrenImpl();
}
TypeRef LocalSpace::computeTypeImpl() { return VoidType::get(context()->evalMode); }
NodeHandle LocalSpace::semanticCheckImpl() {
    // Compute type first
    computeType();

    // Semantic check each of the children
    for (auto child : children()) {
        child.semanticCheck(); // Ignore possible errors
    }
    Feather_checkEvalMode(handle);
    return *this;
}

REGISTER_NODE_KIND_IMPL(GlobalConstructAction);
GlobalConstructAction GlobalConstructAction::create(const Location& loc, NodeHandle action) {
    GlobalConstructAction res =
            createNode<GlobalConstructAction>(nkFeatherGlobalConstructAction, loc);
    res.setChildren(NodeRange{action});
    return res;
}
GlobalConstructAction::GlobalConstructAction(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherGlobalConstructAction);
}
NodeHandle GlobalConstructAction::action() const { return children()[0]; }
NodeHandle GlobalConstructAction::semanticCheckImpl() {
    NodeHandle act = action();
    if (!act.semanticCheck())
        return {};

    handle->type = VoidType::get(context()->evalMode);

    // For CT construct actions, evaluate them asap
    if (Feather_isCt(act)) {
        Nest_ctEval(act);
        return Nop::create(location());
    }
    return *this;
}

REGISTER_NODE_KIND_IMPL(GlobalDestructAction);
GlobalDestructAction GlobalDestructAction::create(const Location& loc, NodeHandle action) {
    GlobalDestructAction res = createNode<GlobalDestructAction>(nkFeatherGlobalDestructAction, loc);
    res.setChildren(NodeRange{action});
    return res;
}
GlobalDestructAction::GlobalDestructAction(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherGlobalDestructAction);
}
NodeHandle GlobalDestructAction::action() const { return children()[0]; }
NodeHandle GlobalDestructAction::semanticCheckImpl() {
    NodeHandle act = action();
    if (!act.semanticCheck())
        return {};

    handle->type = VoidType::get(context()->evalMode);

    // We never CT evaluate global destruct actions
    if (Feather_isCt(act)) {
        return Nop::create(location());
    }
    return *this;
}

REGISTER_NODE_KIND_IMPL(ScopeDestructAction);
ScopeDestructAction ScopeDestructAction::create(const Location& loc, NodeHandle action) {
    ScopeDestructAction res = createNode<ScopeDestructAction>(nkFeatherScopeDestructAction, loc);
    res.setChildren(NodeRange{action});
    return res;
}
ScopeDestructAction::ScopeDestructAction(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherScopeDestructAction);
}
NodeHandle ScopeDestructAction::action() const { return children()[0]; }
NodeHandle ScopeDestructAction::semanticCheckImpl() {
    NodeHandle act = action();
    if (!act.semanticCheck())
        return {};

    handle->type = VoidType::get(context()->evalMode);
    return *this;
}

REGISTER_NODE_KIND_IMPL(TempDestructAction);
TempDestructAction TempDestructAction::create(const Location& loc, NodeHandle action) {
    TempDestructAction res = createNode<TempDestructAction>(nkFeatherTempDestructAction, loc);
    res.setChildren(NodeRange{action});
    return res;
}
TempDestructAction::TempDestructAction(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherTempDestructAction);
}
NodeHandle TempDestructAction::action() const { return children()[0]; }
NodeHandle TempDestructAction::semanticCheckImpl() {
    NodeHandle act = action();
    if (!act.semanticCheck())
        return {};

    handle->type = VoidType::get(context()->evalMode);
    return *this;
}

REGISTER_NODE_KIND_IMPL(ChangeMode);
ChangeMode ChangeMode::create(const Location& loc, NodeHandle child, EvalMode mode) {
    ChangeMode res = createNode<ChangeMode>(nkFeatherChangeMode, loc);
    res.setChildren(NodeRange{child});
    res.setProperty(propEvalMode, (int)mode);
    return res;
}
ChangeMode::ChangeMode(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherChangeMode);
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
void ChangeMode::setContextForChildrenImpl() {
    auto curMode = mode();
    EvalMode newMode = curMode != modeUnspecified ? curMode : context()->evalMode;
    handle->childrenContext = Nest_mkChildContext(context(), newMode);
    NodeHandle::setContextForChildrenImpl();
}
NodeHandle ChangeMode::semanticCheckImpl() {
    NodeHandle exp = child();

    // Make sure we are allowed to change the mode
    EvalMode baseMode = context()->evalMode;
    auto curMode = mode();
    EvalMode newMode = curMode != modeUnspecified ? curMode : baseMode;
    if (newMode == modeUnspecified)
        REP_INTERNAL(location(), "Cannot change the mode to Unspecified");
    if (newMode == modeRt && baseMode != modeRt)
        REP_ERROR_RET(nullptr, location(), "Cannot change mode to RT in a CT context (%1%)") %
                baseMode;

    if (!exp)
        REP_INTERNAL(location(), "No node specified as child to a ChangeMode node");

    return exp.semanticCheck();
}
const char* ChangeMode::toStringImpl() {
    ostringstream os;
    os << "changeMode(" << child() << ", " << mode() << ")";
    return dupString(os.str().c_str());
}

DeclNode::DeclNode(Node* n)
    : NodeHandle(n) {}
StringRef DeclNode::name() const { return getCheckPropertyString("name"); }
EvalMode DeclNode::mode() const { return (EvalMode)getCheckPropertyInt(propEvalMode); }
void DeclNode::setNameAndMode(StringRef name, EvalMode mode) {
    setProperty("name", name);
    setProperty(propEvalMode, modeUnspecified);
}

REGISTER_NODE_KIND_IMPL(FunctionDecl);
FunctionDecl FunctionDecl::create(const Location& loc, StringRef name, NodeHandle resType,
        NodeRange params, NodeHandle body) {
    FunctionDecl res = createNode<FunctionDecl>(nkFeatherDeclFunction, loc);
    res.setChildren(NodeRange{resType, body});
    res.addChildren(params);
    res.setNameAndMode(name, modeUnspecified);
    res.setProperty("callConvention", (int)ccC);

    // Make sure all the nodes given as parameters have the right kind
    for (NodeHandle param : params) {
        if (param.explanation().kind() != nkFeatherDeclVar)
            REP_INTERNAL(param.location(), "Node %1% must be a parameter") % param;
    }

    return res;
}
FunctionDecl::FunctionDecl(Node* n)
    : DeclNode(n) {
    REQUIRE_NODE_KIND(n, nkFeatherDeclFunction);
}
NodeHandle FunctionDecl::resTypeNode() const { return children()[0]; }
NodeRange FunctionDecl::parameters() const { return children().skip(2); }
NodeHandle FunctionDecl::body() const { return children()[1]; }
CallConvention FunctionDecl::callConvention() const {
    return (CallConvention)getCheckPropertyInt("callConvention");
}
void FunctionDecl::addParameter(DeclNode param, bool insertInFront) {
    if (param.explanation().kind() != nkFeatherDeclVar)
        REP_INTERNAL(param.location(), "Node %1% must be a parameter") % param;

    if (insertInFront)
        Nest_insertNodeIntoArray(&handle->children, 2, param);
    else
        Nest_appendNodeToArray(&handle->children, param);
}
void FunctionDecl::setResultType(NodeHandle resType) {
    childrenM()[0] = resType;
    resType.setContext(childrenContext());
}
void FunctionDecl::setContextForChildrenImpl() {
    // If we don't have a children context, create one
    if (!handle->childrenContext)
        handle->childrenContext =
                Nest_mkChildContextWithSymTab(context(), handle, Feather_effectiveEvalMode(handle));

    NodeHandle::setContextForChildrenImpl();

    Feather_addToSymTab(handle);
}
TypeRef FunctionDecl::computeTypeImpl() {
    if (name().empty())
        REP_ERROR_RET(nullptr, location(), "No name given to function declaration");

    // We must have a result type
    TypeRef resType = resTypeNode().computeType();
    if (!resType)
        REP_ERROR_RET(nullptr, location(), "No result type given to function %1%") % name();

    NodeRange params = parameters();

    vector<TypeRef> subTypes;
    subTypes.reserve(params.size() + 1);
    subTypes.push_back(resType);

    // Get the type for all the parameters
    for (auto param : params) {
        if (!param)
            REP_ERROR_RET(nullptr, location(), "Invalid parameter");
        if (!param.computeType())
            return {};
        subTypes.push_back(param.type());
    }

    return FunctionType::get(&subTypes[0], subTypes.size(), Feather_effectiveEvalMode(handle));
}
NodeHandle FunctionDecl::semanticCheckImpl() {
    // Make sure the type is computed
    if (!computeType())
        return {};

    // Semantically check all the parameters
    for (auto param : parameters()) {
        if (!param.semanticCheck())
            return {};
    }

    // Semantically check the body, if we have one
    auto body = this->body();
    if (body)
        body.semanticCheck(); // Ignore possible errors

    // Do we have some valid instructions in the body?
    // Could we mark this function as having an empty body?
    auto resultType = resTypeNode();
    bool noRetValue = !resultType || !resultType.type()->hasStorage;
    bool couldInline = !hasProperty(propNativeName) || !hasProperty(propNoInline);
    if (noRetValue && couldInline && body && !canHaveInstructions(body)) {
        setProperty(propEmptyBody, 1);
    }

    // TODO (function): Check that all the paths return a value
    return *this;
}
const char* FunctionDecl::toStringImpl() {
    ostringstream os;
    os << name();
    if (type()) {
        auto params = parameters();
        TypeBase resultType = resTypeNode().type();
        if (hasProperty(propResultParam)) {
            resultType = params[0].type();
            resultType = removeRef(TypeWithStorage(resultType));
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

REGISTER_NODE_KIND_IMPL(StructDecl);
StructDecl StructDecl::create(const Location& loc, StringRef name, NodeRange fields) {
    StructDecl res = createNode<StructDecl>(nkFeatherDeclClass, loc);
    res.setChildren(fields);
    res.setNameAndMode(name, modeUnspecified);

    // Make sure all the nodes given as fields have the right kind
    for (NodeHandle field : fields) {
        if (field.explanation().kind() != nkFeatherDeclVar)
            REP_INTERNAL(field.location(), "Node %1% must be a field") % field;
    }

    return res;
}
StructDecl::StructDecl(Node* n)
    : DeclNode(n) {
    REQUIRE_NODE_KIND(n, nkFeatherDeclClass);
}
NodeRange StructDecl::fields() const { return children(); }
void StructDecl::setContextForChildrenImpl() {
    // If we don't have a children context, create one
    if (!handle->childrenContext)
        handle->childrenContext =
                Nest_mkChildContextWithSymTab(context(), handle, Feather_effectiveEvalMode(handle));

    NodeHandle::setContextForChildrenImpl();

    Feather_addToSymTab(handle);
}
TypeRef StructDecl::computeTypeImpl() {
    if (name().empty())
        REP_ERROR_RET(nullptr, location(), "No name given to struct");

    // Compute the type for all the fields
    for (auto field : children()) {
        // Ignore errors from children
        field.computeType();
    }
    return DataType::get(*this, 0, Feather_effectiveEvalMode(handle));
}
NodeHandle StructDecl::semanticCheckImpl() {
    if (!computeType())
        return {};

    // Semantically check all the fields
    for (auto field : children())
        field.semanticCheck(); // Ignore possible errors
    return *this;
}

REGISTER_NODE_KIND_IMPL(VarDecl);
VarDecl VarDecl::create(const Location& loc, StringRef name, NodeHandle typeNode) {
    VarDecl res = createNode<VarDecl>(nkFeatherDeclVar, loc);
    res.setChildren(NodeRange{typeNode});
    res.setNameAndMode(name, modeUnspecified);
    res.setProperty("alignment", 0);
    return res;
}
VarDecl::VarDecl(Node* n)
    : DeclNode(n) {
    REQUIRE_NODE_KIND(n, nkFeatherDeclVar);
}
NodeHandle VarDecl::typeNode() const { return children()[0]; }
void VarDecl::setContextForChildrenImpl() {
    NodeHandle::setContextForChildrenImpl();
    Feather_addToSymTab(handle);
}
TypeRef VarDecl::computeTypeImpl() {
    // Make sure the variable has a type
    auto tn = typeNode();
    if (!tn.computeType())
        return {};

    // Adjust the mode of the type
    return Feather_adjustMode(tn.type(), context(), location());
}
NodeHandle VarDecl::semanticCheckImpl() {
    if (!computeType())
        return {};

    // Make sure that the type has storage
    if (!type()->hasStorage)
        REP_ERROR_RET(nullptr, location(), "Variable type has no storage (%1%)") % type();

    NodeHandle datatypeDecl = type()->referredNode;
    if (!datatypeDecl)
        REP_ERROR_RET(nullptr, location(), "Invalid type for variable: %1%") % type();

    datatypeDecl.computeType(); // Make sure the type of the decl is computed
    return *this;
}

REGISTER_NODE_KIND_IMPL(CtValueExp);
CtValueExp CtValueExp::create(const Location& loc, TypeBase type, StringRef data) {
    CtValueExp res = createNode<CtValueExp>(nkFeatherExpCtValue, loc);
    res.setProperty("valueType", type);
    res.setProperty("valueData", data);
    return res;
}
CtValueExp::CtValueExp(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpCtValue);
}
TypeBase CtValueExp::valueType() const { return getCheckPropertyType("valueType"); }
StringRef CtValueExp::valueData() const { return getCheckPropertyString("valueData"); }
NodeHandle CtValueExp::semanticCheckImpl() {
    // Check the type
    if (!handle->type)
        handle->type = valueType();
    if (!type() || !type()->hasStorage)
        REP_ERROR_RET(nullptr, location(),
                "Type specified for Ct Value cannot be used at compile-time (%1%)") %
                type();

    // Make sure data size matches the size reported by the type
    unsigned valueSize = Nest_sizeOf(type());
    StringRef data = valueData();
    if (valueSize != data.size()) {
        REP_ERROR_RET(nullptr, location(),
                "Read value size (%1%) differs from declared size of the value (%2%) - type: %3%") %
                data.size() % valueSize % type();
    }

    handle->type = TypeBase(handle->type).changeMode(modeCt, location());
    return *this;
}
const char* CtValueExp::toString() {
    TypeRef t = type();
    if (!t)
        t = valueType();
    if (!t)
        return "CtValue";
    ostringstream os;
    os << "CtValue(" << t << ": ";

    StringRef valueDataStr = valueData();

    StringRef nativeName = t->hasStorage ? Feather_nativeName(t) : StringRef{};
    if (0 == strcmp(t->description, "Type/ct")) {
        auto t = extractValue<TypeRef>(valueDataStr);
        os << t;
    } else if (nativeName && t->numReferences == 0) {
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

REGISTER_NODE_KIND_IMPL(NullExp);
NullExp NullExp::create(const Location& loc, NodeHandle typeNode) {
    NullExp res = createNode<NullExp>(nkFeatherExpNull, loc);
    res.setChildren(NodeRange{typeNode});
    return res;
}
NullExp::NullExp(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpNull);
}
NodeHandle NullExp::typeNode() const { return children()[0]; }
NodeHandle NullExp::semanticCheckImpl() {
    TypeRef t = typeNode().computeType();
    if (!t)
        return {};

    // Make sure that the type is a reference
    if (!t->hasStorage)
        REP_ERROR_RET(
                nullptr, location(), "Null node should have a type with storage (cur type: %1%") %
                t;
    if (t->numReferences == 0)
        REP_ERROR_RET(
                nullptr, location(), "Null node should have a reference type (cur type: %1%)") %
                t;

    handle->type = Feather_adjustMode(t, context(), location());
    return *this;
}
const char* NullExp::toStringImpl() {
    ostringstream os;
    os << "Null(" << type() << ")";
    return dupString(os.str().c_str());
}

REGISTER_NODE_KIND_IMPL(VarRefExp);
VarRefExp VarRefExp::create(const Location& loc, VarDecl varDecl) {
    REQUIRE_NODE(loc, varDecl);

    VarRefExp res = createNode<VarRefExp>(nkFeatherExpVarRef, loc);
    res.setReferredNodes(NodeRange{varDecl});
    return res;
}
VarRefExp::VarRefExp(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpVarRef);
}
VarDecl VarRefExp::varDecl() const { return VarDecl(referredNodes()[0]); }
NodeHandle VarRefExp::semanticCheckImpl() {
    VarDecl var = varDecl();
    ASSERT(var);
    if (!var.computeType())
        return {};
    if (isField(var))
        REP_INTERNAL(location(), "VarRef used on a field (%1%). Use FieldRef instead") % var.name();
    if (!var.type()->hasStorage)
        REP_ERROR_RET(
                nullptr, location(), "Variable type doesn't have a storage type (type: %1%)") %
                var.type();
    handle->type = Feather_adjustMode(LValueType::get(var.type()), context(), location());
    Feather_checkEvalModeWithExpected(handle, var.type()->mode);
    return *this;
}
const char* VarRefExp::toStringImpl() {
    ostringstream os;
    os << "VarRef(" << varDecl().name() << ")";
    return dupString(os.str().c_str());
}

REGISTER_NODE_KIND_IMPL(FieldRefExp);
FieldRefExp FieldRefExp::create(const Location& loc, NodeHandle obj, VarDecl fieldDecl) {
    REQUIRE_NODE(loc, obj);
    REQUIRE_NODE(loc, fieldDecl);
    FieldRefExp res = createNode<FieldRefExp>(nkFeatherExpFieldRef, loc);
    res.setChildren(NodeRange{obj});
    res.setReferredNodes(NodeRange{fieldDecl});
    return res;
}
FieldRefExp::FieldRefExp(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpFieldRef);
}
NodeHandle FieldRefExp::object() const { return children()[0]; }
VarDecl FieldRefExp::fieldDecl() const { return VarDecl(referredNodes()[0]); }
NodeHandle FieldRefExp::semanticCheckImpl() {
    NodeHandle obj = object();
    VarDecl field = fieldDecl();
    ASSERT(obj);
    ASSERT(field);

    // Semantic check the object node - make sure it's a reference to a data type
    if (!obj.semanticCheck())
        return {};
    ASSERT(obj.type());
    if (!obj.type()->hasStorage || obj.type()->numReferences != 1)
        REP_ERROR_RET(nullptr, location(),
                "Field access should be done on a reference to a data type (type: %1%)") %
                obj.type();
    NodeHandle structNode = obj.type()->referredNode;
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
        REP_ERROR_RET(nullptr, location(), "Field '%1%' not found when accessing object") %
                field.name();

    // Set the correct type for this node
    ASSERT(field.type());
    ASSERT(field.type()->hasStorage);
    handle->type = LValueType::get(field.type());
    EvalMode mode = Feather_combineMode(obj.type()->mode, context()->evalMode);
    handle->type = TypeBase(handle->type).changeMode(mode, location());
    return *this;
}
const char* FieldRefExp::toStringImpl() {
    ostringstream os;
    os << "FieldRef(" << fieldDecl().name() << ")";
    return dupString(os.str().c_str());
}

REGISTER_NODE_KIND_IMPL(FunRefExp);
FunRefExp FunRefExp::create(const Location& loc, FunctionDecl funDecl, NodeHandle resType) {
    REQUIRE_NODE(loc, funDecl);
    REQUIRE_NODE(loc, resType);
    FunRefExp res = createNode<FunRefExp>(nkFeatherExpFunRef, loc);
    res.setChildren(NodeRange{resType});
    res.setReferredNodes(NodeRange{funDecl});
    return res;
}
FunRefExp::FunRefExp(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpFunRef);
}
FunctionDecl FunRefExp::funDecl() const { return FunctionDecl(referredNodes()[0]); }
NodeHandle FunRefExp::resTypeNode() const { return children()[0]; }
NodeHandle FunRefExp::semanticCheckImpl() {
    NodeHandle resType = resTypeNode();
    FunctionDecl fun = funDecl();
    if (!resType.computeType())
        return {};

    if (!fun.computeType())
        return {};
    handle->type = Feather_adjustMode(resType.type(), context(), location());
    return *this;
}
const char* FunRefExp::toStringImpl() {
    ostringstream os;
    os << "FunRef(" << funDecl() << ")";
    return dupString(os.str().c_str());
}

REGISTER_NODE_KIND_IMPL(FunCallExp);
FunCallExp FunCallExp::create(const Location& loc, FunctionDecl funDecl, NodeRange args) {
    REQUIRE_NODE(loc, funDecl);
    FunCallExp res = createNode<FunCallExp>(nkFeatherExpFunCall, loc);
    res.setChildren(args);
    res.setReferredNodes(NodeRange{funDecl});
    return res;
}
FunCallExp::FunCallExp(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpFunCall);
}
FunctionDecl FunCallExp::funDecl() const { return FunctionDecl(referredNodes()[0]); }
NodeRange FunCallExp::arguments() const { return children(); }
NodeHandle FunCallExp::semanticCheckImpl() {
    FunctionDecl fun = funDecl();

    // Make sure the function declaration is has a valid type
    if (!fun.computeType())
        return {};

    // Check argument count
    int numParameters = fun.parameters().size();
    int numArgs = arguments().size();
    if (numArgs != numParameters)
        REP_ERROR_RET(
                nullptr, location(), "Invalid function call: expecting %1% parameters, given %2%") %
                numParameters % numArgs;

    // Semantic check the arguments
    // Also check that their type matches the corresponding type from the function decl
    bool allParamsAreCtAvailable = true;
    for (int i = 0; i < numArgs; ++i) {
        NodeHandle arg = arguments()[i];

        // Semantically check the argument
        if (!arg.semanticCheck())
            return {};
        if (!Feather_isCt(arg))
            allParamsAreCtAvailable = false;

        // Compare types
        TypeRef argType = arg.type();
        TypeRef paramType = fun.parameters()[i].type();
        if (!sameTypeIgnoreMode(argType, paramType))
            REP_ERROR_RET(nullptr, arg.location(),
                    "Invalid function call; parameter %1% expected type %2%, given %3%") %
                    fun.parameters()[i] % paramType % argType;
    }

    // CT availability checks
    EvalMode curMode = context()->evalMode;
    EvalMode calledFunMode = Feather_effectiveEvalMode(fun);
    ASSERT(curMode != modeUnspecified);
    ASSERT(calledFunMode != modeUnspecified);
    if (calledFunMode == modeCt && curMode != modeCt && !allParamsAreCtAvailable) {
        REP_ERROR(location(),
                "Not all arguments are compile-time, when calling a compile time function");
        REP_INFO(fun.location(), "See called function");
        return {};
    }

    // Get the type from the function decl
    handle->type = fun.resTypeNode().type();

    // Handle autoCt case
    if (allParamsAreCtAvailable && handle->type->mode == modeRt && fun.hasProperty(propAutoCt)) {
        handle->type = TypeBase(handle->type).changeMode(modeCt, location());
    }

    // Make sure we yield a type with the right mode
    handle->type = Feather_adjustMode(handle->type, context(), location());

    // Check if this node can have some meaningful instructions generated for it
    bool noRetValue = !handle->type->hasStorage;
    if (noRetValue && !canHaveInstructions(*this)) {
        setProperty(propEmptyBody, 1);
    }

    Feather_checkEvalModeWithExpected(*this, calledFunMode);
    return *this;
}
const char* FunCallExp::toStringImpl() {
    ostringstream os;
    os << "FunCall-" << funDecl().name() << "(" << arguments() << ")";
    return dupString(os.str().c_str());
}

REGISTER_NODE_KIND_IMPL(MemLoadExp);
MemLoadExp MemLoadExp::create(const Location& loc, NodeHandle address) {
    REQUIRE_NODE(loc, address);
    MemLoadExp res = createNode<MemLoadExp>(nkFeatherExpMemLoad, loc);
    res.setChildren(NodeRange{address});
    res.setProperty("alignment", 0);
    res.setProperty("volatile", 0);
    res.setProperty("atomicOrdering", (int)atomicNone);
    res.setProperty("singleThreaded", 0);
    return res;
}
MemLoadExp::MemLoadExp(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpMemLoad);
}
NodeHandle MemLoadExp::address() const { return children()[0]; }
NodeHandle MemLoadExp::semanticCheckImpl() {
    NodeHandle exp = address();

    // Semantic check the argument
    if (!exp.semanticCheck())
        return {};

    // Check if the type of the argument is a ref
    if (!exp.type()->hasStorage || exp.type()->numReferences == 0)
        REP_ERROR_RET(nullptr, location(), "Cannot load from a non-reference (%1%, type: %2%)") %
                exp % exp.type();

    // Check flags
    auto ordering = (AtomicOrdering)getCheckPropertyInt("atomicOrdering");
    if (ordering == atomicRelease)
        REP_ERROR_RET(nullptr, location(), "Cannot use atomic release with a load instruction");
    if (ordering == atomicAcquireRelease)
        REP_ERROR_RET(
                nullptr, location(), "Cannot use atomic acquire-release with a load instruction");

    // Remove the 'ref' from the type and get the base type
    handle->type = removeRef(TypeWithStorage(exp.type()));
    handle->type = Feather_adjustMode(handle->type, context(), location());
    return *this;
}

REGISTER_NODE_KIND_IMPL(MemStoreExp);
MemStoreExp MemStoreExp::create(const Location& loc, NodeHandle value, NodeHandle address) {
    REQUIRE_NODE(loc, value);
    REQUIRE_NODE(loc, address);
    MemStoreExp res = createNode<MemStoreExp>(nkFeatherExpMemStore, loc);
    res.setChildren(NodeRange{value, address});
    res.setProperty("alignment", 0);
    res.setProperty("volatile", 0);
    res.setProperty("atomicOrdering", (int)atomicNone);
    res.setProperty("singleThreaded", 0);
    return res;
}
MemStoreExp::MemStoreExp(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpMemStore);
}
NodeHandle MemStoreExp::value() const { return children()[0]; }
NodeHandle MemStoreExp::address() const { return children()[1]; }
NodeHandle MemStoreExp::semanticCheckImpl() {
    NodeHandle value = this->value();
    NodeHandle address = this->address();
    ASSERT(value);
    ASSERT(address);

    // Semantic check the arguments
    if (!value.semanticCheck() || !address.semanticCheck())
        return {};

    // Check if the type of the address is a ref
    if (!address.type()->hasStorage || address.type()->numReferences == 0)
        REP_ERROR_RET(nullptr, location(),
                "The address of a memory store is not a reference, nor VarRef nor FieldRef (type: "
                "%1%)") %
                address.type();
    TypeRef baseAddressType = removeRef(TypeWithStorage(address.type()));

    // Check the equivalence of types
    if (!sameTypeIgnoreMode(value.type(), baseAddressType)) {
        // Try again, getting rid of l-values
        if (!sameTypeIgnoreMode(lvalueToRefIfPresent(value.type()), baseAddressType))
            REP_ERROR_RET(nullptr, location(),
                    "The type of the value doesn't match the type of the address in a memory store "
                    "(%1% != %2%)") %
                    value.type() % baseAddressType;
    }

    // The resulting type is Void
    handle->type = VoidType::get(address.type()->mode);
    return *this;
}

REGISTER_NODE_KIND_IMPL(BitcastExp);
BitcastExp BitcastExp::create(const Location& loc, NodeHandle destType, NodeHandle exp) {
    REQUIRE_NODE(loc, destType);
    REQUIRE_NODE(loc, exp);
    BitcastExp res = createNode<BitcastExp>(nkFeatherExpBitcast, loc);
    res.setChildren(NodeRange{exp, destType});
    return res;
}
BitcastExp::BitcastExp(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpBitcast);
}
NodeHandle BitcastExp::destTypeNode() const { return children()[1]; }
NodeHandle BitcastExp::expression() const { return children()[0]; }
NodeHandle BitcastExp::semanticCheckImpl() {
    if (!expression().semanticCheck())
        return {};
    TypeRef tDest = destTypeNode().computeType();
    if (!tDest)
        return {};

    // Make sure both types have storage
    TypeRef srcType = expression().type();
    if (!srcType->hasStorage)
        REP_ERROR_RET(
                nullptr, location(), "The source of a bitcast is not a type with storage (%1%)") %
                srcType;
    if (!tDest->hasStorage)
        REP_ERROR_RET(nullptr, location(),
                "The destination type of a bitcast is not a type with storage (%1%)") %
                tDest;

    // Make sure both types are references
    if (srcType->numReferences == 0)
        REP_ERROR_RET(nullptr, location(), "The source of a bitcast is not a reference (%1%)") %
                srcType;
    if (tDest->numReferences == 0)
        REP_ERROR_RET(
                nullptr, location(), "The destination type of a bitcast is not a reference (%1%)") %
                tDest;

    handle->type = Feather_adjustMode(tDest, context(), location());
    return *this;
}
const char* BitcastExp::toStringImpl() {
    ostringstream os;
    if (destTypeNode().type())
        os << "Bitcast(" << destTypeNode().type() << ", " << expression() << ")";
    else
        os << "Bitcast(type(" << destTypeNode() << "), " << expression() << ")";
    return dupString(os.str().c_str());
}

REGISTER_NODE_KIND_IMPL(ConditionalExp);
ConditionalExp ConditionalExp::create(
        const Location& loc, NodeHandle cond, NodeHandle alt1, NodeHandle alt2) {
    REQUIRE_NODE(loc, cond);
    REQUIRE_NODE(loc, alt1);
    REQUIRE_NODE(loc, alt2);
    ConditionalExp res = createNode<ConditionalExp>(nkFeatherExpConditional, loc);
    res.setChildren(NodeRange{cond, alt1, alt2});
    return res;
}
ConditionalExp::ConditionalExp(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpConditional);
}
NodeHandle ConditionalExp::condition() const { return children()[0]; }
NodeHandle ConditionalExp::alt1() const { return children()[1]; }
NodeHandle ConditionalExp::alt2() const { return children()[2]; }
NodeHandle ConditionalExp::semanticCheckImpl() {
    NodeHandle& cond = childrenM()[0];
    NodeHandle alt1 = this->alt1();
    NodeHandle alt2 = this->alt2();

    // Semantic check the condition
    if (!cond.semanticCheck())
        return {};

    // Check that the type of the condition is 'Testable'
    if (!Feather_isTestable(cond))
        REP_ERROR_RET(nullptr, cond.location(),
                "The condition of the conditional expression is not Testable");

    // Dereference the condition as much as possible
    while (cond.type() && cond.type()->numReferences > 0) {
        cond = MemLoadExp::create(cond.location(), cond);
        cond.setContext(childrenContext());
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
        REP_ERROR_RET(nullptr, location(),
                "The types of the alternatives of a conditional must be equal (%1% != %2%)") %
                alt1.type() % alt2.type();

    EvalMode mode = Feather_combineModeBottom(alt1.type()->mode, cond.type()->mode);
    mode = Feather_combineMode(mode, context()->evalMode);
    handle->type = TypeBase(alt1.type()).changeMode(mode, location());
    return *this;
}

REGISTER_NODE_KIND_IMPL(IfStmt);
IfStmt IfStmt::create(const Location& loc, NodeHandle cond, NodeHandle thenC, NodeHandle elseC) {
    REQUIRE_NODE(loc, cond);
    IfStmt res = createNode<IfStmt>(nkFeatherStmtIf, loc);
    res.setChildren(NodeRange{cond, thenC, elseC});
    return res;
}
IfStmt::IfStmt(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherStmtIf);
}
NodeHandle IfStmt::condition() const { return children()[0]; }
NodeHandle IfStmt::thenClause() const { return children()[1]; }
NodeHandle IfStmt::elseClause() const { return children()[2]; }
void IfStmt::setContextForChildrenImpl() {
    handle->childrenContext = Nest_mkChildContextWithSymTab(context(), *this, modeUnspecified);
    NodeHandle::setContextForChildrenImpl();
}
NodeHandle IfStmt::semanticCheckImpl() {
    NodeHandle condition = this->condition();
    NodeHandle thenClause = this->thenClause();
    NodeHandle elseClause = this->elseClause();

    // The resulting type is Void
    handle->type = VoidType::get(context()->evalMode);

    // Semantic check the condition
    if (!condition.semanticCheck())
        return {};

    // Check that the type of the condition is 'Testable'
    if (!Feather_isTestable(condition))
        REP_ERROR_RET(nullptr, condition.location(),
                "The condition of the if is not Testable; type=%1%") %
                condition.type();

    // Dereference the condition as much as possible
    while (condition.type() && condition.type()->numReferences > 0) {
        condition = MemLoadExp::create(condition.location(), condition);
        condition.setContext(childrenContext());
        if (!condition.semanticCheck())
            return {};
    }
    childrenM()[0] = condition;
    // TODO (if): Remove this dereference from here

    if (Feather_nodeEvalMode(*this) == modeCt) {
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
            return Nop::create(location());
    }

    // Semantic check the clauses
    if (thenClause && !thenClause.semanticCheck())
        return {};
    if (elseClause && !elseClause.semanticCheck())
        return {};
    return *this;
}

REGISTER_NODE_KIND_IMPL(WhileStmt);
WhileStmt WhileStmt::create(
        const Location& loc, NodeHandle cond, NodeHandle body, NodeHandle step) {
    REQUIRE_NODE(loc, cond);
    REQUIRE_NODE(loc, body);
    WhileStmt res = createNode<WhileStmt>(nkFeatherStmtWhile, loc);
    res.setChildren(NodeRange{cond, step, body});
    return res;
}
WhileStmt::WhileStmt(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherStmtWhile);
}
NodeHandle WhileStmt::condition() const { return children()[0]; }
NodeHandle WhileStmt::body() const { return children()[2]; }
NodeHandle WhileStmt::step() const { return children()[1]; }
void WhileStmt::setContextForChildrenImpl() {
    handle->childrenContext = Nest_mkChildContextWithSymTab(context(), *this, modeUnspecified);
    CompilationContext* condContext = Feather_nodeEvalMode(*this) == modeCt
                                              ? Nest_mkChildContext(context(), modeCt)
                                              : childrenContext();

    condition().setContext(condContext);
    if (step())
        step().setContext(condContext);
    if (body())
        body().setContext(childrenContext());
}
NodeHandle WhileStmt::semanticCheckImpl() {
    NodeHandle condition = this->condition();
    NodeHandle step = this->step();
    NodeHandle body = this->body();

    // Semantic check the condition
    if (!condition.semanticCheck())
        return {};
    if (step && !step.computeType())
        return {};

    // Check that the type of the condition is 'Testable'
    if (!Feather_isTestable(condition))
        REP_ERROR_RET(nullptr, condition.location(), "The condition of the while is not Testable");

    // Dereference the condition as much as possible
    while (condition.type() && condition.type()->numReferences > 0) {
        condition = MemLoadExp::create(condition.location(), condition);
        condition.setContext(childrenContext());
        if (!condition.semanticCheck())
            return {};
    }
    childrenM()[0] = condition;
    // TODO (while): Remove this dereference from here

    if (Feather_nodeEvalMode(*this) == modeCt) {
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
                curBody.setContext(context());
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
        NodeHandle res = NodeList::create(location(), all(result), true);
        Nest_freeNodeArray(result);
        return res;
    }

    // Semantic check the body and the step
    if (body && !body.semanticCheck())
        return {};
    if (step && !step.semanticCheck())
        return {};

    // The resulting type is Void
    handle->type = VoidType::get(context()->evalMode);
    return *this;
}

REGISTER_NODE_KIND_IMPL(BreakStmt);
BreakStmt BreakStmt::create(const Location& loc) {
    BreakStmt res = createNode<BreakStmt>(nkFeatherStmtBreak, loc);
    res.setProperty("loop", NodeHandle{});
    return res;
}
BreakStmt::BreakStmt(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherStmtBreak);
}
NodeHandle BreakStmt::semanticCheckImpl() {
    // Get the outer-most loop from the context
    NodeHandle loop = Feather_getParentLoop(context());
    if (!loop)
        REP_ERROR_RET(nullptr, location(), "Break found outside any loop");
    setProperty("loop", loop);

    // The resulting type is Void
    handle->type = VoidType::get(context()->evalMode);
    return *this;
}

REGISTER_NODE_KIND_IMPL(ContinueStmt);
ContinueStmt ContinueStmt::create(const Location& loc) {
    ContinueStmt res = createNode<ContinueStmt>(nkFeatherStmtContinue, loc);
    res.setProperty("loop", NodeHandle{});
    return res;
}
ContinueStmt::ContinueStmt(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherStmtContinue);
}
NodeHandle ContinueStmt::semanticCheckImpl() {
    // Get the outer-most loop from the context
    NodeHandle loop = Feather_getParentLoop(context());
    if (!loop)
        REP_ERROR_RET(nullptr, location(), "Continue found outside any loop");
    setProperty("loop", loop);

    // The resulting type is Void
    handle->type = VoidType::get(context()->evalMode);
    return *this;
}

REGISTER_NODE_KIND_IMPL(ReturnStmt);
ReturnStmt ReturnStmt::create(const Location& loc, NodeHandle exp) {
    ReturnStmt res = createNode<ReturnStmt>(nkFeatherStmtReturn, loc);
    res.setChildren(NodeRange{exp});
    res.setProperty("parentFun", NodeHandle{});
    return res;
}
ReturnStmt::ReturnStmt(Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherStmtReturn);
}
NodeHandle ReturnStmt::expression() const { return children()[0]; }
NodeHandle ReturnStmt::semanticCheckImpl() {
    NodeHandle exp = expression();

    // If we have an expression argument, semantically check it
    if (exp && !exp.semanticCheck())
        return {};

    // Get the parent function of this return
    NodeHandle parentFun = Feather_getParentFun(context());
    if (!parentFun)
        REP_ERROR_RET(nullptr, location(), "Return found outside any function");
    TypeRef resultType = FunctionDecl(parentFun).resTypeNode().type();
    ASSERT(resultType);
    setProperty("parentFun", parentFun);

    // If the return has an expression, check that has the same type as the function result type
    if (exp) {
        if (!sameTypeIgnoreMode(exp.type(), resultType))
            REP_ERROR_RET(nullptr, location(),
                    "Returned expression's type is not the same as function's return type");
    } else {
        // Make sure that the function has a void return type
        if (resultType->typeKind != typeKindVoid)
            REP_ERROR_RET(nullptr, location(),
                    "You must return something in a function that has non-Void result type");
    }

    // The resulting type is Void
    handle->type = VoidType::get(context()->evalMode);
    return *this;
}

} // namespace Feather
