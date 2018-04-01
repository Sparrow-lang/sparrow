#include <StdInc.h>
#include "Mods.h"

#include "Helpers/DeclsHelpers.h"

#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Utils/NodeUtils.hpp"

using namespace SprFrontend;

void ModPublic_beforeComputeType(Modifier*, Node* node) { setAccessType(node, publicAccess); }

void ModProtected_beforeComputeType(Modifier*, Node* node) { setAccessType(node, protectedAccess); }

void ModPrivate_beforeComputeType(Modifier*, Node* node) { setAccessType(node, privateAccess); }

void ModCt_beforeSetContext(Modifier*, Node* node) { Feather_setEvalMode(node, modeCt); }

void ModRt_beforeSetContext(Modifier*, Node* node) { Feather_setEvalMode(node, modeRt); }

void ModAutoCt_beforeSetContext(Modifier*, Node* node) {
    if (node->nodeKind == nkSparrowDeclSprFunction) {
        Nest_setPropertyInt(node, propAutoCt, 1);
        Feather_setEvalMode(node, modeRt);
    } else
        REP_INTERNAL(node->location,
                "'autoCt' modifier can be applied only to functions (applied to %1%)") %
                Nest_nodeKindName(node);
}

void ModCtGeneric_beforeComputeType(Modifier*, Node* node) {
    /// Check to apply only to classes or functions
    if (node->nodeKind != nkSparrowDeclSprFunction) {
        REP_ERROR(node->location,
                "ctGeneric modifier can be applied only to functions (applied to %1%)") %
                Nest_nodeKindName(node);
        return;
    }

    Feather_setEvalMode(node, modeCt);
    Nest_setPropertyInt(node, propCtGeneric, 1);
}

struct _NativeMod {
    Modifier base;
    StringRef name;
};

void ModNative_beforeComputeType(Modifier* mod, Node* node) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* nativeMod = (_NativeMod*)mod;
    Nest_setPropertyString(node, propNativeName, nativeMod->name);
}

void ModConvert_beforeComputeType(Modifier*, Node* node) {
    /// Check to apply only to constructors
    if (node->nodeKind != nkSparrowDeclSprFunction || Feather_getName(node) != "ctor")
        REP_INTERNAL(node->location,
                "convert modifier can be applied only to constructors (applied to %1%)") %
                Nest_nodeKindName(node);

    Nest_setPropertyInt(node, propConvert, 1);
}

void ModNoDefault_beforeComputeType(Modifier*, Node* node) {
    /// Check to apply only to classes or functions
    if (node->nodeKind != nkSparrowDeclSprFunction && node->nodeKind != nkSparrowDeclSprDatatype)
        REP_INTERNAL(node->location,
                "noDefault modifier can be applied only to classes or methods (applied to %1%)") %
                Nest_nodeKindName(node);

    Nest_setPropertyInt(node, propNoDefault, 1);
}

void ModInitCtor_beforeComputeType(Modifier*, Node* node) {
    /// Check to apply only to classes
    if (node->nodeKind != nkSparrowDeclSprDatatype) {
        REP_ERROR(node->location,
                "initCtor modifier can be applied only to classes (applied to %1%)") %
                Nest_nodeKindName(node);
        return;
    }

    Nest_setPropertyInt(node, propGenerateInitCtor, 1);
}

void ModMacro_beforeComputeType(Modifier*, Node* node) {
    /// Check to apply only to functions
    if (node->nodeKind != nkSparrowDeclSprFunction) {
        REP_ERROR(node->location,
                "macro modifier can be applied only to functions (applied to %1%)") %
                Nest_nodeKindName(node);
        return;
    }

    Nest_setPropertyInt(node, propMacro, 1);
    Nest_setPropertyInt(node, propCtGeneric, 1);
}

void ModNoInline_beforeComputeType(Modifier*, Node* node) {
    if (node->nodeKind != nkSparrowDeclSprFunction)
        REP_INTERNAL(node->location,
                "noInline modifier can be applied only to functions (applied to %1%)") %
                Nest_nodeKindName(node);

    Nest_setPropertyInt(node, propNoInline, 1);
}

Modifier _publicMod = {modTypeBeforeComputeType, &ModPublic_beforeComputeType};
Modifier _protectedMod = {modTypeBeforeComputeType, &ModProtected_beforeComputeType};
Modifier _privateMod = {modTypeBeforeComputeType, &ModPrivate_beforeComputeType};
Modifier _ctMod = {modTypeBeforeSetContext, &ModCt_beforeSetContext};
Modifier _rtMod = {modTypeBeforeSetContext, &ModRt_beforeSetContext};
Modifier _autoCtMod = {modTypeBeforeSetContext, &ModAutoCt_beforeSetContext};
Modifier _ctGenericMod = {modTypeBeforeComputeType, &ModCtGeneric_beforeComputeType};
Modifier _convertMod = {modTypeBeforeComputeType, &ModConvert_beforeComputeType};
Modifier _noDefaultMod = {modTypeBeforeComputeType, &ModNoDefault_beforeComputeType};
Modifier _initCtorMod = {modTypeBeforeComputeType, ModInitCtor_beforeComputeType};
Modifier _macroMod = {modTypeBeforeComputeType, &ModMacro_beforeComputeType};
Modifier _noInlineMod = {modTypeBeforeComputeType, &ModNoInline_beforeComputeType};

Modifier* SprFe_getPublicMod() { return &_publicMod; }
Modifier* SprFe_getProtectedMod() { return &_protectedMod; }
Modifier* SprFe_getPrivateMod() { return &_privateMod; }
Modifier* SprFe_getCtMod() { return &_ctMod; }
Modifier* SprFe_getRtMod() { return &_rtMod; }
Modifier* SprFe_getAutoCtMod() { return &_autoCtMod; }
Modifier* SprFe_getCtGenericMod() { return &_ctGenericMod; }
Modifier* SprFe_getConvertMod() { return &_convertMod; }
Modifier* SprFe_getNoDefaultMod() { return &_noDefaultMod; }
Modifier* SprFe_getInitCtorMod() { return &_initCtorMod; }
Modifier* SprFe_getMacroMod() { return &_macroMod; }
Modifier* SprFe_getNoInlineMod() { return &_noInlineMod; }

Modifier* SprFe_getNativeMod(StringRef name) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* res = (_NativeMod*)alloc(sizeof(_NativeMod), allocGeneral);
    res->name = dup(name);
    res->base.modifierType = modTypeBeforeComputeType;
    res->base.modifierFun = &ModNative_beforeComputeType;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    return (Modifier*)res;
}

bool SprFe_isEvalModeMod(Modifier* mod) {
    return mod == &_ctMod || mod == &_rtMod;
}
