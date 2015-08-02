#include <StdInc.h>
#include "SprTypeTraits.h"
#include "DeclsHelpers.h"
#include "Overload.h"
#include "StdDef.h"
#include "Impl/Callable.h"
#include <NodeCommonsCpp.h>
#include <Nodes/Decls/SprConcept.h>
#include <SparrowFrontendTypes.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Util/TypeTraits.h>

#include <Nest/Common/Tuple.h>

using namespace SprFrontend;
using namespace Feather;

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
        return ConversionResult(combine(first.conversionType(), second.conversionType()), [=](DynNode* src) -> DynNode* {
            DynNode* src1 = first.apply(src);
            src1->setContext(src->context());
            return second.apply(src1);
        }, first.contextDependent() || second.contextDependent());
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

        TypeRef srcTypeNew = Feather::changeTypeMode(srcType, destMode);
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
                srcTypeNew = Feather::removeLValueIfPresent(srcTypeNew);

                res = ConversionResult(convDirect, [=](DynNode* src) -> DynNode* {
                    return (DynNode*) mkMemLoad(src->location(), src->node());
                });
            }
            if ( srcTypeNew->numReferences > 0 )
                return convNone;
        }

        return combine(res, cachedCanConvertImpl(context, flags, srcTypeNew, destType));
    }
    
    /// concept: #C@N -> Concept@N
    ConversionResult checkConvertToAuto(CompilationContext* /*context*/, int /*flags*/, TypeRef srcType, TypeRef destType)
    {
        if ( destType->typeKind != typeKindConcept )
            return convNone;
        if ( srcType->typeKind == typeKindConcept || !srcType->hasStorage )
            return convNone;

        if ( srcType->typeKind != typeKindLValue && srcType->numReferences == destType->numReferences )
        {
            SprConcept* concept = conceptOfType(destType);

            // If we have a concept, check if the type fulfills the concept
            if ( concept )
                return concept->isFulfilled(srcType) ? convConcept : convNone;
            else
                return convConcept;
        }
        else
            return convNone;
    }

    /// concept: Concept1@N -> Concept2@N
    ConversionResult checkConceptToConcept(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType)
    {
        if ( srcType->typeKind != typeKindConcept || destType->typeKind != typeKindConcept
            || srcType->numReferences != destType->numReferences )
            return convNone;

        SprConcept* srcConcept = conceptOfType(srcType);
        if ( !srcConcept )
            return convNone;

        // If we have a concept, check if the type fulfills the concept
        TypeRef srcBaseConceptType = srcConcept->baseConceptType();
        return cachedCanConvertImpl(context, flags, srcBaseConceptType, destType);
    }
    

    // direct: lv(T) -> U, if T-> U or addRef(T) -> U (take best alternative)
    ConversionResult checkLValueToNormal(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType)
    {
        if ( srcType->typeKind == typeKindLValue && destType->typeKind != typeKindLValue )
        {
            TypeRef t2 = Feather::lvalueToRef(srcType);
            TypeRef t1 = Feather::removeRef(t2);

            // First check conversion without reference
            ConversionResult res1 = ConversionResult(convDirect, [=](DynNode* src) -> DynNode* {
                return (DynNode*) mkMemLoad(src->location(), src->node());
            });
            ConversionResult c1 = combine(res1, cachedCanConvertImpl(context, flags | flagDontAddReference, t1, destType));
            if ( c1 )
                return c1;

            // Now try to convert to reference
            ConversionResult res2 = ConversionResult(convImplicit, [=](DynNode* src) -> DynNode* {
                return (DynNode*) mkBitcast(src->location(), mkTypeNode(src->location(), t2), src->node());
            });
            return combine(res2, cachedCanConvertImpl(context, flags, t2, destType));
        }
        return convNone;
    }

    // implicit: #Null -> U, if isRef(U) and isStorage(U)
    ConversionResult checkNullToReference(CompilationContext* /*context*/, int /*flags*/, TypeRef srcType, TypeRef destType)
    {
        if ( !StdDef::typeNull
            || !Feather::isSameTypeIgnoreMode(srcType, StdDef::typeNull)
            || !destType->hasStorage || destType->numReferences == 0 )
            return convNone;

        return ConversionResult(convImplicit, [=](DynNode* src) -> DynNode* {
            return (DynNode*) mkNull(src->location(), mkTypeNode(src->location(), destType));
        });
    }

    // implicit: #C@N -> U, if #C@(N-1) -> U
    ConversionResult checkDereference(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType)
    {
        if ( srcType->typeKind != typeKindData || srcType->numReferences == 0 )
            return convNone;

        TypeRef t = removeRef(srcType);

        ConversionResult res = ConversionResult(convImplicit, [=](DynNode* src) -> DynNode* {
            return (DynNode*) mkMemLoad(src->location(), src->node());
        });
        return combine(res, cachedCanConvertImpl(context, flags | flagDontAddReference, t, destType));
    }
    
    // implicit:  #C@0 -> U, if #C@1 -> U
    ConversionResult checkAddReference(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType)
    {
        if ( srcType->typeKind != typeKindData || srcType->numReferences > 0 )
            return convNone;

        TypeRef baseDataType = addRef(srcType);

        ConversionResult res(convImplicit, [=](DynNode* src) -> DynNode* {
            Node* var = Feather::mkVar(src->location(), "$tmpForRef", mkTypeNode(src->location(), srcType));
            Node* varRef = mkVarRef(src->location(), var);
            Node* store = mkMemStore(src->location(), src->node(), varRef);
            Node* cast = mkBitcast(src->location(), mkTypeNode(src->location(), baseDataType), varRef);
            return (DynNode*) mkNodeList(src->location(), {var, store, cast});
        });
        return combine(res, cachedCanConvertImpl(context, flags | flagDontAddReference, baseDataType, destType));
    }
    
    // T => U, if U has a conversion ctor for T
    ConversionResult checkConversionCtor(CompilationContext* context, int flags, TypeRef srcType, TypeRef destType)
    {
        if ( !destType->hasStorage )
            return convNone;

        Class* destClass = classForType(destType);
        destClass->computeType();

        // Try to convert srcType to lv destClass
        if ( !selectConversionCtor(context, destClass, destType->mode, srcType, nullptr, nullptr) )
            return convNone;

        TypeRef t = destClass->type();
        EvalMode destMode = t->mode;
        if ( destMode == modeRtCt )
            destMode = srcType->mode;
        t = Feather::changeTypeMode(t, destMode);
        TypeRef resType = getLValueType(t);

        bool contextDependent = false;  // TODO (convert): This should be context dependent for private ctors

        ConversionResult res = ConversionResult(convCustom, [=](DynNode* src) -> DynNode* {
            DynNode* refToClass = createTypeNode(src->context(), src->location(), getDataType(destClass->node()));
            return (DynNode*) mkChangeMode(src->location(), mkFunApplication(src->location(), refToClass->node(), NodeVector(1, src->node())), destMode);
        }, contextDependent);
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

//         cerr << "Checking conversion: " << srcType << " -> " << destType << endl;
//         if ( srcType->toString() == "RetroRange[ListRange[Int[rtct]]]" && destType->toString() == "concept[BidirRange]" )
//         {
//             const char* s = "put a breakpoint here";
//             (void) s;
//         }

        // Direct: Types are the same?
        c = checkSameType(context, flags, srcType, destType);
        if ( c )
            return c;

        // Direct: Ct to non-ct
        c = checkChangeMode(context, flags, srcType, destType);
        if ( c )
            return c;

        // Direct: Type with storage to concept
        c = checkConvertToAuto(context, flags, srcType, destType);
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
        typedef Tuple3<TypeRef, TypeRef, int> KeyType;
        static unordered_map<KeyType, ConversionResult> convMap;

        // Try to find the conversion in the map
        KeyType key(srcType, destType, flags);
        auto it = convMap.find(key);
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
        if ( !res.contextDependent() )
            convMap.insert(make_pair(key, res));

        return res;
    }
}

ConversionResult::ConversionResult(ConversionType convType, const ConversionFun& fun, bool contextDependent)
    : convType_(convType)
    , convFun_(fun)
    , contextDependent_(contextDependent)
{
}

Node* ConversionResult::apply(Node* src) const
{
    return apply((DynNode*) src)->node();
}

Node* ConversionResult::apply(CompilationContext* context, Node* src) const
{
    return apply(context, (DynNode*) src)->node();
}

DynNode* ConversionResult::apply(DynNode* src) const
{
    return convType_ != convNone && convFun_
        ? convFun_(src)
        : src;
}

DynNode* ConversionResult::apply(CompilationContext* context, DynNode* src) const
{
    DynNode* res = convType_ != convNone && convFun_
        ? convFun_(src)
        : src;
    res->setContext(context);
    return res;
}


ConversionResult SprFrontend::canConvertType(CompilationContext* context, Nest::TypeRef srcType, Nest::TypeRef destType, ConversionFlags flags)
{
    return cachedCanConvertImpl(context, flags, srcType, destType);
}

ConversionResult SprFrontend::canConvert(Node* arg, TypeRef destType, ConversionFlags flags)
{
    return canConvert((DynNode*) arg, destType, flags);
}
ConversionResult SprFrontend::canConvert(DynNode* arg, TypeRef destType, ConversionFlags flags)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.convert", "Type conversion checking");

    ASSERT(arg);
    arg->computeType();
    TypeRef srcType = arg->type();
    ASSERT(srcType);
    ASSERT(destType);

    return cachedCanConvertImpl(arg->context(), flags, srcType, destType);
}
