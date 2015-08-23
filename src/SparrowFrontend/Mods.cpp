#include <StdInc.h>
#include "Mods.h"

#include <Feather/Util/Decl.h>

#include <Nest/Common/Alloc.h>
#include <Nest/Common/Diagnostic.h>
#include <Nest/Intermediate/Modifier.h>

using namespace SprFrontend;


void ModStatic_beforeComputeType(Modifier*, Node* node)
{
    if ( node->nodeKind != nkSparrowDeclSprVariable && node->nodeKind != nkSparrowDeclSprFunction )
        REP_INTERNAL(node->location, "Static modifier can be applied only to variables and functions inside classes");

    Nest::setProperty(node, propIsStatic, 1);
}

void ModCt_beforeSetContext(Modifier*, Node* node)
{
    Feather::setEvalMode(node, modeCt);
}

void ModRt_beforeSetContext(Modifier*, Node* node)
{
    Feather::setEvalMode(node, modeRt);
}

void ModRtCt_beforeSetContext(Modifier*, Node* node)
{
    Feather::setEvalMode(node, modeRtCt);
}

void ModAutoCt_beforeSetContext(Modifier*, Node* node)
{
    using namespace Feather;

    if ( node->nodeKind == nkSparrowDeclSprFunction )
    {
        setProperty(node, propAutoCt, 1);
        setEvalMode(node, modeRtCt);
    }
    else
        REP_INTERNAL(node->location, "'autoCt' modifier can be applied only to functions");
}

void ModCtGeneric_beforeComputeType(Modifier*, Node* node)
{
    /// Check to apply only to classes or functions
    if ( node->nodeKind != nkSparrowDeclSprFunction )
        REP_ERROR(node->location, "ctGeneric modifier can be applied only to functions");

    Feather::setEvalMode(node, modeCt);
    Nest::setProperty(node, propCtGeneric, 1);
}

struct _NativeMod {
    Modifier base;
    const char* name;
};

void ModNative_beforeComputeType(Modifier* mod, Node* node)
{
    _NativeMod* nativeMod = (_NativeMod*) mod;
    Nest::setProperty(node, Feather::propNativeName, nativeMod->name);
}

void ModConvert_beforeComputeType(Modifier*, Node* node)
{
    /// Check to apply only to constructors
    if ( node->nodeKind != nkSparrowDeclSprFunction || Feather::getName(node) != "ctor" )
        REP_INTERNAL(node->location, "convert modifier can be applied only to constructors");

    Nest::setProperty(node, propConvert, 1);
}

void ModNoDefault_beforeComputeType(Modifier*, Node* node)
{
    /// Check to apply only to classes or functions
    if ( node->nodeKind != nkSparrowDeclSprFunction && node->nodeKind != nkSparrowDeclSprClass )
        REP_INTERNAL(node->location, "noDefault modifier can be applied only to classes or methods");

    Nest::setProperty(node, propNoDefault, 1);
}

void ModInitCtor_beforeComputeType(Modifier*, Node* node)
{
    /// Check to apply only to classes
    if ( node->nodeKind != nkSparrowDeclSprClass )
        REP_ERROR(node->location, "initCtor modifier can be applied only to classes");
    
    Nest::setProperty(node, propGenerateInitCtor, 1);
}

void ModMacro_beforeComputeType(Modifier*, Node* node)
{
    /// Check to apply only to functions
    if ( node->nodeKind != nkSparrowDeclSprFunction )
        REP_ERROR(node->location, "macro modifier can be applied only to functions");
    
    Nest::setProperty(node, propMacro, 1);
    Nest::setProperty(node, propCtGeneric, 1);
}

void ModNoInline_beforeComputeType(Modifier*, Node* node)
{
    if ( node->nodeKind != nkSparrowDeclSprFunction )
        REP_INTERNAL(node->location, "noInline modifier can be applied only to functions");

    Nest::setProperty(node, Feather::propNoInline, 1);
}


Modifier _staticMod = { NULL,NULL, &ModStatic_beforeComputeType,NULL, NULL,NULL };
Modifier _ctMod = { &ModCt_beforeSetContext,NULL, NULL,NULL, NULL,NULL };
Modifier _rtMod = { &ModRt_beforeSetContext,NULL, NULL,NULL, NULL,NULL };
Modifier _rtCtMod = { &ModRtCt_beforeSetContext,NULL, NULL,NULL, NULL,NULL };
Modifier _autoCtMod = { &ModAutoCt_beforeSetContext,NULL, NULL,NULL, NULL,NULL };
Modifier _ctGenericMod = { NULL,NULL, &ModCtGeneric_beforeComputeType,NULL, NULL,NULL };
Modifier _convertMod = { NULL,NULL, &ModConvert_beforeComputeType,NULL, NULL,NULL };
Modifier _noDefaultMod = { NULL,NULL, &ModNoDefault_beforeComputeType,NULL, NULL,NULL };
Modifier _initCtorMod = { NULL,NULL, ModInitCtor_beforeComputeType,NULL, NULL,NULL };
Modifier _macroMod = { NULL,NULL, &ModMacro_beforeComputeType,NULL, NULL,NULL };
Modifier _noInlineMod = { NULL,NULL, &ModNoInline_beforeComputeType,NULL, NULL,NULL };

Modifier* SprFe_getStaticMod()
{
    return &_staticMod;
}
Modifier* SprFe_getCtMod()
{
    return &_ctMod;
}
Modifier* SprFe_getRtMod()
{
    return &_rtMod;
}
Modifier* SprFe_getRtCtMod()
{
    return &_rtCtMod;
}
Modifier* SprFe_getAutoCtMod()
{
    return &_autoCtMod;
}
Modifier* SprFe_getCtGenericMod()
{
    return &_ctGenericMod;
}
Modifier* SprFe_getConvertMod()
{
    return &_convertMod;
}
Modifier* SprFe_getNoDefaultMod()
{
    return &_noDefaultMod;
}
Modifier* SprFe_getInitCtorMod()
{
    return &_initCtorMod;
}
Modifier* SprFe_getMacroMod()
{
    return &_macroMod;
}
Modifier* SprFe_getNoInlineMod()
{
    return &_noInlineMod;
}

Modifier* SprFe_getNativeMod(const char* name)
{
    _NativeMod* res = (_NativeMod*) alloc(sizeof(_NativeMod), allocGeneral);
    res->name = dupString(name);
    res->base.beforeComputeType = &ModNative_beforeComputeType;
    return (Modifier*) res;
}

bool SprFe_isEvalModeMod(Modifier* mod)
{
    return mod == &_ctMod
        || mod == &_rtMod
        || mod == &_rtCtMod
        ;
}
