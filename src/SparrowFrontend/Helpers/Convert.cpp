#include <StdInc.h>
#include "SprTypeTraits.h"
#include "DeclsHelpers.h"
#include "Overload.h"
#include "StdDef.h"
#include "Impl/Callable.h"
#include <NodeCommonsCpp.h>
#include <SparrowFrontendTypes.h>
#include <Helpers/Generics.h>

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Utils/Tuple.hpp"

using namespace SprFrontend;

ConversionType SprFrontend::combine(ConversionType lhs, ConversionType rhs)
{
    if ( lhs == convConcept && rhs == convImplicit )
        return convConceptWithImplicit;
    if ( rhs == convConcept && lhs == convImplicit )
        return convConceptWithImplicit;
    return worstConv(lhs, rhs);
}

ConversionType SprFrontend::worstConv(ConversionType lhs, ConversionType rhs)
{
    return (ConversionType) min(lhs, rhs);
}

ConversionType SprFrontend::bestConv(ConversionType lhs, ConversionType rhs)
{
    return (ConversionType) max(lhs, rhs);
}


namespace
{
    ConversionResult cachedCanConvertImpl(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType);

    ConversionResult combine(const ConversionResult& first, const ConversionResult& second)
    {
        const SourceCode* sc = nullptr;
        if ( first.sourceCode() ) {
            sc = first.sourceCode();
            if ( second.sourceCode() && second.sourceCode() != sc )
                return convNone;
        }
        else
            sc = second.sourceCode();

        return ConversionResult(combine(first.conversionType(), second.conversionType()), [=](Node* src) -> Node* {
            Node* src1 = first.apply(src);
            Nest_setContext(src1, src->context);
            return second.apply(src1);
        }, sc);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Conversion checks
    //

    // direct: T -> T
    ConversionResult checkSameType(CompilationContext* /*context*/, int /*flags*/, TypeRef srcType, TypeRef destType)
    {
        return srcType == destType ? convDirect : convNone;
    }

    // direct: T/X -> U/Y, if X!=Y and Y!=ct and (T==ct => noRef(T)==0) and hasMeaning(T/Y) and T/Y -> U/Y
    ConversionResult checkChangeMode(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType)
    {
        if ( !srcType->hasStorage )
            return convNone;

        EvalMode srcMode = srcType->mode;
        EvalMode destMode = destType->mode;

        // Don't do anything if the modes are the same; can't convert from non-CT to CT
        if ( srcMode == destMode || destMode == modeCt )
            return convNone;

        TypeRef srcTypeNew = Feather_checkChangeTypeMode(srcType, destMode, NOLOC);
        if ( srcTypeNew == srcType )
            return convNone;

        ConversionResult res(convDirect);
        if ( srcMode == modeCt )
        {
            // If we are converting away from CT, make sure we are allowed to do this
            if ( !srcType->canBeUsedAtRt )
                return convNone;

            // Cannot convert references from CT to RT; still we allow l-value conversions
            if ( srcTypeNew->typeKind == typeKindLValue )
            {
                if ( srcTypeNew->numReferences > 1 )
                    return convNone;
                srcTypeNew = Feather_removeLValueIfPresent(srcTypeNew);

                res = ConversionResult(convDirect, [=](Node* src) -> Node* {
                    return Feather_mkMemLoad(src->location, src);
                });
            }
            if ( srcTypeNew->numReferences > 0 )
                return convNone;
        }

        return combine(res, cachedCanConvertImpl(context, flags, srcTypeNew, destType));
    }

    /// concept: #C@N -> Concept@N
    ConversionResult checkConvertToConcept(CompilationContext* /*context*/, int /*flags*/, TypeRef srcType, TypeRef destType)
    {
        if ( destType->typeKind != typeKindConcept )
            return convNone;
        if ( srcType->typeKind == typeKindConcept || !srcType->hasStorage )
            return convNone;

        bool isOk = false;

        if ( srcType->typeKind != typeKindLValue && srcType->numReferences == destType->numReferences )
        {
            Node* concept = conceptOfType(destType);
            if ( !concept ) {
                isOk = true;
            }
            else {
                // If we have a concept, check if the type fulfills the concept
                if ( concept->nodeKind == nkSparrowDeclSprConcept ) {
                    isOk = conceptIsFulfilled(concept, srcType);
                }

                // If we have a generic, check if the type is generated from the generic
                if ( concept->nodeKind == nkSparrowDeclGenericClass ) {
                    isOk = typeGeneratedFromGeneric(concept, srcType);
                }
            }

        }

        return isOk ? convConcept : convNone;
    }

    /// concept: Concept1@N -> Concept2@N
    ConversionResult checkConceptToConcept(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType)
    {
        if ( srcType->typeKind != typeKindConcept || destType->typeKind != typeKindConcept
            || srcType->numReferences != destType->numReferences )
            return convNone;

        Node* srcConcept = conceptOfType(srcType);
        if ( !srcConcept )
            return convNone;


        // If we have a concept, check if the type fulfills the concept
        TypeRef srcBaseConceptType = baseConceptType(srcConcept);
        srcBaseConceptType = Feather_checkChangeTypeMode(srcBaseConceptType, srcType->mode, srcConcept->location);
        return cachedCanConvertImpl(context, flags, srcBaseConceptType, destType);
    }


    // direct: lv(T) -> U, if T-> U or Feather_addRef(T) -> U (take best alternative)
    ConversionResult checkLValueToNormal(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType)
    {
        if ( srcType->typeKind == typeKindLValue && destType->typeKind != typeKindLValue )
        {
            TypeRef t2 = Feather_lvalueToRef(srcType);
            TypeRef t1 = Feather_removeRef(t2);

            // First check conversion without reference
            ConversionResult res1 = ConversionResult(convDirect, [=](Node* src) -> Node* {
                return Feather_mkMemLoad(src->location, src);
            });
            ConversionResult c1 = combine(res1, cachedCanConvertImpl(context, flags | flagDontAddReference, t1, destType));
            if ( c1 )
                return c1;

            // Now try to convert to reference
            ConversionResult res2 = ConversionResult(convImplicit, [=](Node* src) -> Node* {
                return Feather_mkBitcast(src->location, Feather_mkTypeNode(src->location, t2), src);
            });
            return combine(res2, cachedCanConvertImpl(context, flags, t2, destType));
        }
        return convNone;
    }

    // implicit: #Null -> U, if isRef(U) and isStorage(U)
    ConversionResult checkNullToReference(CompilationContext* /*context*/, int /*flags*/, TypeRef srcType, TypeRef destType)
    {
        if ( !StdDef::typeNull
            || !Feather_isSameTypeIgnoreMode(srcType, StdDef::typeNull)
            || !destType->hasStorage || destType->numReferences == 0 )
            return convNone;

        return ConversionResult(convImplicit, [=](Node* src) -> Node* {
            return Feather_mkNull(src->location, Feather_mkTypeNode(src->location, destType));
        });
    }

    // implicit: #C@N -> U, if #C@(N-1) -> U
    ConversionResult checkDereference(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType)
    {
        if ( srcType->typeKind != typeKindData || srcType->numReferences == 0 )
            return convNone;

        TypeRef t = Feather_removeRef(srcType);

        ConversionResult res = ConversionResult(convImplicit, [=](Node* src) -> Node* {
            return Feather_mkMemLoad(src->location, src);
        });
        return combine(res, cachedCanConvertImpl(context, flags | flagDontAddReference, t, destType));
    }

    // implicit:  #C@0 -> U, if #C@1 -> U
    ConversionResult checkAddReference(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType)
    {
        if ( srcType->typeKind != typeKindData || srcType->numReferences > 0 )
            return convNone;

        TypeRef baseDataType = Feather_addRef(srcType);

        ConversionResult res(convImplicit, [=](Node* src) -> Node* {
            Node* var = Feather_mkVar(src->location, fromCStr("$tmpForRef"), Feather_mkTypeNode(src->location, srcType));
            Node* varRef = Feather_mkVarRef(src->location, var);
            Node* store = Feather_mkMemStore(src->location, src, varRef);
            Node* cast = Feather_mkBitcast(src->location, Feather_mkTypeNode(src->location, baseDataType), varRef);
            return Feather_mkNodeList(src->location, fromIniList({var, store, cast}));
        });
        return combine(res, cachedCanConvertImpl(context, flags | flagDontAddReference, baseDataType, destType));
    }

    // T => U, if U has a conversion ctor for T
    ConversionResult checkConversionCtor(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType)
    {
        if ( !destType->hasStorage )
            return convNone;

        Node* destClass = Feather_classForType(destType);
        if ( !Nest_computeType(destClass) )
            return convNone;

        // Try to convert srcType to lv destClass
        if ( !selectConversionCtor(context, destClass, destType->mode, srcType, nullptr, nullptr) )
            return convNone;

        // Check access
        if ( !canAccessNode(destClass, context->sourceCode) )
            return convNone;

        // If the class is not public, store the current source code for this conversion
        // This conversion is not ok in all contexts
        SourceCode* sourceCode = nullptr;
        if ( !isPublic(destClass) )
            sourceCode = context->sourceCode;

        TypeRef t = destClass->type;
        EvalMode destMode = t->mode;
        if ( destMode == modeRtCt )
            destMode = srcType->mode;
        t = Feather_checkChangeTypeMode(t, destMode, NOLOC);
        TypeRef resType = Feather_getLValueType(t);

        ConversionResult res = ConversionResult(convCustom, [=](Node* src) -> Node* {
            Node* refToClass = createTypeNode(src->context, src->location, Feather_getDataType(destClass, 0, modeRtCt));
            return Feather_mkChangeMode(src->location, mkFunApplication(src->location, refToClass, fromIniList({src})), destMode);
        }, sourceCode);
        return combine(res, cachedCanConvertImpl(context, flags | flagDontCallConversionCtor, resType, destType));
    }

    // The main canConvert method
    ConversionResult canConvertImpl(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType)
    {
        ASSERT(srcType);
        ASSERT(destType);
        ConversionResult c(convNone);

        bool canCallCvtCtor = 0 == (flags & flagDontCallConversionCtor);
        flags |= flagDontCallConversionCtor;    // Don't call conversion ctor in any other conversions

        // cerr << "Checking conversion: " << srcType << " -> " << destType << endl;
        // if ( 0 == strcmp(srcType->description, "Tuple[NumericRangeInc[Int/rtct]/rtct, NumericRangeInc[Int/rtct]/rtct]")
        //     && 0 == strcmp(destType->description, "#TupleType") )
        // {
        //     const char* s = "put a breakpoint here";
        //     (void) s;
        // }

        // Direct: Types are the same?
        c = checkSameType(context, flags, srcType, destType);
        if ( c )
            return c;

        // Direct: Ct to non-ct
        c = checkChangeMode(context, flags, srcType, destType);
        if ( c )
            return c;

        // Direct: Type with storage to concept
        c = checkConvertToConcept(context, flags, srcType, destType);
        if ( c )
            return c;

        // Direct: Concept to another concept
        c = checkConceptToConcept(context, flags, srcType, destType);
        if ( c )
            return c;


        // Direct: If we have a l-value, convert into a reference
        c = checkLValueToNormal(context, flags, srcType, destType);     // Recursive call
        if ( c )
            return c;

        // Implicit: Null to reference
        c = checkNullToReference(context, flags, srcType, destType);    // Recursive call
        if ( c )
            return c;

        // Implicit: Reference to non reference?
        c = checkDereference(context, flags, srcType, destType);        // Recursive call
        if ( c )
            return c;

        // Implicit: Non-reference to reference
        if ( (flags & flagDontAddReference) == 0 )
        {
            c = checkAddReference(context, flags, srcType, destType);   // Recursive call
            if ( c )
                return c;
        }


        // Custom: Conversion with conversion constructor
        if ( canCallCvtCtor )
        {
            c = checkConversionCtor(context, flags, srcType, destType);             // Recursive call
            if ( c )
                return c;
        }

        return convNone;
    }

    // Method that checks for available conversions; use a cache for speeding up search
    ConversionResult cachedCanConvertImpl(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType)
    {
        typedef Tuple4<TypeRef, TypeRef, int, const SourceCode*> KeyType;
        static unordered_map<KeyType, ConversionResult> convMap;

        // Try to find the conversion in the map -- first, try without a source code
        KeyType key(srcType, destType, flags, nullptr);
        auto it = convMap.find(key);
        if ( it != convMap.end() )
            return it->second;
        // Now try with a source code
        key = KeyType(srcType, destType, flags, context->sourceCode);
        it = convMap.find(key);
        if ( it != convMap.end() )
            return it->second;

        // Compute the value normally
        ConversionResult res = canConvertImpl(context, flags, srcType, destType);

//         cerr << "Conv result: " << srcType << " -> " << destType << " (" << flags << ") : ";
//         switch ( res.conversionType() )
//         {
//         case convNone:              cerr << "None"; break;
//         case convCustom:            cerr << "Custom"; break;
//         case convConceptWithImplicit:  cerr << "Concept + implicit"; break;
//         case convConcept:              cerr << "Concept"; break;
//         case convImplicit:          cerr << "Implicit"; break;
//         case convDirect:            cerr << "Direct"; break;
//         default:                    cerr << "???"; break;
//         }
//         cerr << "\n";

        // Put the result in the cache, if not context dependent
        key._4 = res.sourceCode();
        convMap.insert(make_pair(key, res));

        return res;
    }
}

ConversionResult::ConversionResult(ConversionType convType, const ConversionFun& fun, const SourceCode* sourceCode)
    : convType_(convType)
    , convFun_(fun)
    , sourceCode_(sourceCode)
{
}

Node* ConversionResult::apply(Node* src) const
{
    return convType_ != convNone && convFun_
        ? convFun_(src)
        : src;
}

Node* ConversionResult::apply(CompilationContext* context, Node* src) const
{
    Node* res = convType_ != convNone && convFun_
        ? convFun_(src)
        : src;
    Nest_setContext(res, context);
    return res;
}


ConversionResult SprFrontend::canConvertType(CompilationContext* context, TypeRef srcType, TypeRef destType, ConversionFlags flags)
{
    return cachedCanConvertImpl(context, flags, srcType, destType);
}

ConversionResult SprFrontend::canConvert(Node* arg, TypeRef destType, ConversionFlags flags)
{
    ASSERT(arg);
    TypeRef srcType = Nest_computeType(arg);
    if ( !srcType )
        return convNone;
    ASSERT(destType);

    return cachedCanConvertImpl(arg->context, flags, srcType, destType);
}
