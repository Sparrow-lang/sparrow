#include <StdInc.h>
#include "Mods.h"

#include "Helpers/DeclsHelpers.h"

#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Utils/cppif/NodeUtils.hpp"

using namespace SprFrontend;

void ModPublic_beforeComputeType(Nest_Modifier*, Node* node) { setAccessType(node, publicAccess); }

void ModProtected_beforeComputeType(Nest_Modifier*, Node* node) {
    setAccessType(node, protectedAccess);
}

void ModPrivate_beforeComputeType(Nest_Modifier*, Node* node) {
    setAccessType(node, privateAccess);
}

void ModCt_beforeSetContext(Nest_Modifier*, Node* node) { Feather_setEvalMode(node, modeCt); }

void ModRt_beforeSetContext(Nest_Modifier*, Node* node) { Feather_setEvalMode(node, modeRt); }

void ModAutoCt_beforeSetContext(Nest_Modifier*, Node* node) {
    if (node->nodeKind == nkSparrowDeclSprFunction) {
        Nest_setPropertyInt(node, propAutoCt, 1);
        Feather_setEvalMode(node, modeRt);
    } else
        REP_INTERNAL(node->location,
                "'autoCt' modifier can be applied only to functions (applied to %1%)") %
                Nest_nodeKindName(node);
}

void ModCtGeneric_beforeComputeType(Nest_Modifier*, Node* node) {
    /// Check to apply only to functions
    if (node->nodeKind != nkSparrowDeclSprFunction) {
        REP_ERROR(node->location,
                "ctGeneric modifier can be applied only to functions (applied to %1%)") %
                Nest_nodeKindName(node);
        return;
    }

    Feather_setEvalMode(node, modeCt);
    Nest_setPropertyInt(node, propCtGeneric, 1);
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct _NativeMod {
    Nest_Modifier base;
    StringRef name;
};

void ModNative_beforeComputeType(Nest_Modifier* mod, Node* node) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* nativeMod = (_NativeMod*)mod;
    Nest_setPropertyString(node, propNativeName, nativeMod->name);
}

void ModConvert_beforeComputeType(Nest_Modifier*, Node* node) {
    /// Check to apply only to constructors and datatypes
    if ((node->nodeKind != nkSparrowDeclSprFunction ||
                Feather_getName(node) != StringRef("ctor")) &&
            node->nodeKind != nkSparrowDeclSprDatatype)
        REP_INTERNAL(node->location, "convert modifier can be applied only to constructors and "
                                     "datatypes (applied to %1%)") %
                Nest_nodeKindName(node);

    Nest_setPropertyInt(node, propConvert, 1);
}

void ModNoDefault_beforeComputeType(Nest_Modifier*, Node* node) {
    /// Check to apply only to datatypes or functions
    if (node->nodeKind != nkSparrowDeclSprFunction && node->nodeKind != nkSparrowDeclSprDatatype)
        REP_INTERNAL(node->location, "noDefault modifier can be applied only to datatypes or "
                                     "functions (applied to %1%)") %
                Nest_nodeKindName(node);

    Nest_setPropertyInt(node, propNoDefault, 1);
}

void ModInitCtor_beforeComputeType(Nest_Modifier*, Node* node) {
    /// Check to apply only to datatypes
    if (node->nodeKind != nkSparrowDeclSprDatatype) {
        REP_ERROR(node->location,
                "initCtor modifier can be applied only to datatypes (applied to %1%)") %
                Nest_nodeKindName(node);
        return;
    }

    Nest_setPropertyInt(node, propGenerateInitCtor, 1);
}

void ModBitCopiable_beforeComputeType(Nest_Modifier*, Node* node) {
    /// Check to apply only to datatypes
    if (node->nodeKind != nkSparrowDeclSprDatatype) {
        REP_ERROR(node->location,
                "bitcopiable modifier can be applied only to datatypes (applied to %1%)") %
                Nest_nodeKindName(node);
        return;
    }

    Nest_setPropertyInt(node, propBitCopiable, 1);
}

void ModAutoBitCopiable_beforeComputeType(Nest_Modifier*, Node* node) {
    /// Check to apply only to datatypes
    if (node->nodeKind != nkSparrowDeclSprDatatype) {
        REP_ERROR(node->location,
                "autoBitcopiable modifier can be applied only to datatypes (applied to %1%)") %
                Nest_nodeKindName(node);
        return;
    }

    Nest_setPropertyInt(node, propAutoBitCopiable, 1);
}

void ModMacro_beforeComputeType(Nest_Modifier*, Node* node) {
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

void ModNoInline_beforeComputeType(Nest_Modifier*, Node* node) {
    if (node->nodeKind != nkSparrowDeclSprFunction)
        REP_INTERNAL(node->location,
                "noInline modifier can be applied only to functions (applied to %1%)") %
                Nest_nodeKindName(node);

    Nest_setPropertyInt(node, propNoInline, 1);
}

Nest_Modifier _publicMod = {modTypeBeforeComputeType, &ModPublic_beforeComputeType};
Nest_Modifier _protectedMod = {modTypeBeforeComputeType, &ModProtected_beforeComputeType};
Nest_Modifier _privateMod = {modTypeBeforeComputeType, &ModPrivate_beforeComputeType};
Nest_Modifier _ctMod = {modTypeBeforeSetContext, &ModCt_beforeSetContext};
Nest_Modifier _rtMod = {modTypeBeforeSetContext, &ModRt_beforeSetContext};
Nest_Modifier _autoCtMod = {modTypeBeforeSetContext, &ModAutoCt_beforeSetContext};
Nest_Modifier _ctGenericMod = {modTypeBeforeComputeType, &ModCtGeneric_beforeComputeType};
Nest_Modifier _convertMod = {modTypeBeforeComputeType, &ModConvert_beforeComputeType};
Nest_Modifier _noDefaultMod = {modTypeBeforeComputeType, &ModNoDefault_beforeComputeType};
Nest_Modifier _initCtorMod = {modTypeBeforeComputeType, ModInitCtor_beforeComputeType};
Nest_Modifier _bitCopiableMod = {modTypeBeforeComputeType, &ModBitCopiable_beforeComputeType};
Nest_Modifier _autoBitCopiableMod = {
        modTypeBeforeComputeType, &ModAutoBitCopiable_beforeComputeType};
Nest_Modifier _macroMod = {modTypeBeforeComputeType, &ModMacro_beforeComputeType};
Nest_Modifier _noInlineMod = {modTypeBeforeComputeType, &ModNoInline_beforeComputeType};

Nest_Modifier* SprFe_getPublicMod() { return &_publicMod; }
Nest_Modifier* SprFe_getProtectedMod() { return &_protectedMod; }
Nest_Modifier* SprFe_getPrivateMod() { return &_privateMod; }
Nest_Modifier* SprFe_getCtMod() { return &_ctMod; }
Nest_Modifier* SprFe_getRtMod() { return &_rtMod; }
Nest_Modifier* SprFe_getAutoCtMod() { return &_autoCtMod; }
Nest_Modifier* SprFe_getCtGenericMod() { return &_ctGenericMod; }
Nest_Modifier* SprFe_getConvertMod() { return &_convertMod; }
Nest_Modifier* SprFe_getNoDefaultMod() { return &_noDefaultMod; }
Nest_Modifier* SprFe_getInitCtorMod() { return &_initCtorMod; }
Nest_Modifier* SprFe_getBitCopiableMod() { return &_bitCopiableMod; }
Nest_Modifier* SprFe_getAutoBitCopiableMod() { return &_autoBitCopiableMod; }
Nest_Modifier* SprFe_getMacroMod() { return &_macroMod; }
Nest_Modifier* SprFe_getNoInlineMod() { return &_noInlineMod; }

Nest_Modifier* SprFe_getNativeMod(StringRef name) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    auto* res = (_NativeMod*)alloc(sizeof(_NativeMod), allocGeneral);
    res->name = name.dup();
    res->base.modifierType = modTypeBeforeComputeType;
    res->base.modifierFun = &ModNative_beforeComputeType;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    return (Nest_Modifier*)res;
}

bool SprFe_isEvalModeMod(Nest_Modifier* mod) { return mod == &_ctMod || mod == &_rtMod; }
