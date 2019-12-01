#include <StdInc.h>
#include "ConvertServiceImpl.h"
#include "SparrowFrontend/Helpers/SprTypeTraits.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"
#include "SparrowFrontend/Helpers/StdDef.h"
#include "SparrowFrontend/NodeCommonsCpp.h"
#include "SparrowFrontend/SparrowFrontendTypes.hpp"
#include "SparrowFrontend/Nodes/SprProperties.h"
#include "SparrowFrontend/Services/IOverloadService.h"
#include "SparrowFrontend/Services/IConceptsService.h"

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Nest/Utils/Tuple.hpp"
#include "Nest/Utils/cppif/SmallVector.hpp"
#include "Nest/Utils/Profiling.h"
#include "Nest/Utils/Reverse.hpp"

#include <utility>

namespace SprFrontend {

using namespace Feather;
using Nest::SmallVector;

ConversionResult ConvertServiceImpl::checkConversion(
        CompilationContext* context, Type srcType, Type destType, ConversionFlags flags) {
    return cachedCheckConversion(context, flags, srcType, destType);
}

ConversionResult ConvertServiceImpl::checkConversion(
        Node* arg, Type destType, ConversionFlags flags) {
    ASSERT(arg);
    Type srcType = Nest_computeType(arg);
    if (!srcType)
        return {};
    ASSERT(destType);

    return cachedCheckConversion(arg->context, flags, srcType, destType);
}

ConversionResult ConvertServiceImpl::checkConversionImpl(
        CompilationContext* context, int flags, Type src, Type dest) {
    ASSERT(src);
    ASSERT(dest);

    // If the types are the same, then we are happy
    if (src == dest)
        return convDirect;

    // If any of the types doesn't have storage, conversion is invalid
    if (!src.hasStorage() || !dest.hasStorage())
        return convNone;

    TypeWithStorage srcS = TypeWithStorage(src);
    TypeWithStorage destS = TypeWithStorage(dest);

    ConversionResult res{convDirect};

    // Check for null conversions
    if (StdDef::clsNull && srcS.referredNode() == StdDef::clsNull) {
        if (isDataLikeType(destS) && destS.numReferences() > 0 &&
                destS.referredNode() != StdDef::clsNull) {
            res.addConversion(convImplicit, ConvAction(ActionType::makeNull, destS));
            return res;
        }
    }

    // Do the types have the same mode?
    if (srcS.mode() == destS.mode()) {
        if (!checkConversionSameMode(res, context, flags, srcS, destS))
            return {};
    } else {
        // The only supported mode conversion is CT->RT
        if (srcS.mode() != modeCt || destS.mode() != modeRt)
            return {};
        // For datatypes conversion, the source type must be usable at RT
        // TODO (types): check MyRange/ct -> #Range, where MyRange is ct-only
        if (!isConceptType(destS) && !srcS.canBeUsedAtRt())
            return {};

        // Disallow conversion of references
        // @T/ct -> @T is disallowed
        // Allowed: T/ct -> @T, @T/ct -> T, T/ct -> T mut, T/ct mut -> T
        int srcRefsBase = srcS.numReferences();
        int destRefsBase = destS.numReferences();
        if (isCategoryType(srcS))
            --srcRefsBase;
        if (isCategoryType(destS))
            --destRefsBase;
        if (srcRefsBase != 0 && destRefsBase != 0)
            return {};

        // If the types have different modes, then we split our conversion in the following:
        //  a) convert 'src' to reach zero-references
        //  b) apply CT-to-RT conversion
        //  c) covert result to 'dest'

        TypeWithStorage src0; // same as 'src', with with zero references
        if (isDataLikeType(srcS))
            src0 = removeAllRefs(srcS);
        else if (isConceptType(srcS)) {
            src0 = ConceptType::get(ConceptType(srcS).decl(), srcS.mode());
        } else
            return {};

        // a) Remove all references from source
        if (srcS != src0)
            if (!checkConversionSameMode(res, context, flags, srcS, src0))
                return {};

        // b) remove CT
        src0 = src0.changeMode(modeRt, NOLOC);
        res.addConversion(convDirect, ConvAction(ActionType::modeCast, src0));

        // c) convert src0 to dest
        if (src0 != destS)
            if (!checkConversionSameMode(res, context, flags, src0, destS))
                return {};
    }

    return res;
}

bool ConvertServiceImpl::checkConversionSameMode(ConversionResult& res, CompilationContext* context,
        int flags, TypeWithStorage src, TypeWithStorage dest) {
    ASSERT(src);
    ASSERT(dest);

    TypeWithStorage destBase = baseType(dest);

    // Is the destination is a concept?
    if (isConceptType(destBase)) {
        return checkConversionToConcept(res, context, flags, src, dest);
    }

    // Treat data-like to data-like conversions
    if (isDataLikeType(dest) && isDataLikeType(src)) {
        return checkDataConversion(
                res, context, flags, TypeWithStorage(src), TypeWithStorage(dest));
    }

    return false;
}

bool ConvertServiceImpl::checkConversionToConcept(ConversionResult& res,
        CompilationContext* context, int flags, TypeWithStorage src, TypeWithStorage dest) {
    ASSERT(src);
    ASSERT(dest);

    TypeWithStorage srcBase = baseType(src);
    TypeWithStorage destBase = baseType(dest);

    // Case 1: concept -> concept
    if (isConceptType(srcBase)) {
        // Check wrapper types
        if (!checkWrapperTypes(res, src, dest))
            return false;

        // Iteratively search the base concept to find our dest type
        src = srcBase;
        while (src != destBase) {
            ConceptDecl conceptNode = ConceptDecl(ConceptType(src).decl());
            if (!conceptNode)
                return false;
            ConceptType baseType = g_ConceptsService->baseConceptType(conceptNode);
            if (!baseType || baseType == src)
                return false; // Not found; cannot convert
            src = baseType.changeMode(src.mode(), conceptNode.location());
        }

        return true;
    }

    // Case 2: data-like -> concept (concept)
    else if (Feather::isDataLikeType(src)) {

        // Treat the destination type kind as data-like
        int destTypeKind = dest.kind();
        if (destTypeKind == typeKindConcept)
            destTypeKind = typeKindData;

        // Check wrapper types
        if (!checkWrapperTypes(res, src, dest))
            return false;

        bool isOk = false;
        Nest::NodeHandle concept = dest.referredNode();
        if (!concept)
            isOk = true;
        // If we have a concept, check if the type fulfills the concept
        else if (concept.kind() == nkSparrowDeclSprConcept) {
            isOk = g_ConceptsService->conceptIsFulfilled(ConceptDecl(concept), src);
        }
        // If we have a generic, check if the type is generated from the generic
        GenericDatatype genericDatatype = concept.kindCast<GenericDatatype>();
        if (genericDatatype) {
            isOk = g_ConceptsService->typeGeneratedFromGeneric(genericDatatype, src);
        }
        if (!isOk)
            return false;

        // Conversion is possible
        res.addConversion(convConcept);
        return true;
    }

    return false;
}

bool ConvertServiceImpl::checkDataConversion(ConversionResult& res, CompilationContext* context,
        int flags, TypeWithStorage src, TypeWithStorage dest) {

    // Precondition: we only support datatype-like conversions
    if (!isDataLikeType(removeCategoryIfPresent(src)))
        return false;

    // Case 1: The datatypes have the same decl
    if (dest.referredNode() == src.referredNode()) {
        // Check wrapper types
        if (!checkWrapperTypes(res, src, dest))
            return false;

        res.addConversion(convDirect);
        return true;
    }

    // Case 2: Custom conversions
    else if (0 == (flags & flagDontCallConversionCtor)) {

        Node* destClass = dest.referredNode();
        if (!destClass)
            return false;

        // If the destination class is not marked as convert, bail out
        // This saves us some processing time
        if (!Nest_hasProperty(destClass, propConvert))
            return false;

        if (!Nest_computeType(destClass))
            return false;

        // Try to convert srcType to mut destClass
        if (!g_OverloadService->selectConversionCtor(context, destClass, dest.mode(), src))
            return false;

        // Check access
        if (!canAccessNode(destClass, context->sourceCode))
            return false;

        // If the class is not public, store the current source code for this conversion
        // This conversion is not ok in all contexts
        Nest_SourceCode* sourceCode = nullptr;
        if (!isPublic(destClass))
            sourceCode = context->sourceCode;

        TypeWithStorage t = destClass->type;
        EvalMode destMode = t.mode();
        if (destMode == modeRt)
            destMode = src.mode();
        t = t.changeMode(destMode, NOLOC);
        TypeWithStorage resType = Feather::isCategoryType(t) ? t : MutableType::get(t);

        res.addConversion(convCustom, ConvAction(ActionType::customCvt, resType), sourceCode);

        // Finally, check the wrapper types
        if (!checkWrapperTypes(res, resType, dest))
            return false;

        return true;
    }

    return false;
}

namespace {

//! A logical type wrapper
enum TypeWrapper {
    twPlain = 0,  //!< T
    twPtr,        //!< Ptr(T), T != cat
    twConst,      //!< Const(T)
    twMutable,    //!< Mutable(T)
    twTemp,       //!< Temp(T)
    twPtrConst,   //!< Ptr(Const(T))
    twPtrMutable, //!< Ptr(Mutable(T))
    twPtrTemp,    //!< Ptr(Temp(T))
};

ostream& operator<<(ostream& os, TypeWrapper tw) {
    switch (tw) {
    case twPlain:
        os << "Plain";
        break;
    case twPtr:
        os << "Ptr";
        break;
    case twConst:
        os << "Const";
        break;
    case twMutable:
        os << "Mutable";
        break;
    case twTemp:
        os << "Temp";
        break;
    case twPtrConst:
        os << "PtrConst";
        break;
    case twPtrMutable:
        os << "PtrMutable";
        break;
    case twPtrTemp:
        os << "PtrTemp";
        break;
    default:
        os << "UnknownWrapper";
    }
    return os;
}

//! Checks if the type wrapper is a ptr-like (with or without cat)
bool isPtr(TypeWrapper t) {
    return t == twPtr || t == twPtrConst || t == twPtrMutable || t == twPtrTemp;
}

//! Decompose the type between a base type and a set of wrapper types
//! Note: we have distinct wrappers for ptr, ptr(const), ptr(mutable) and ptr(temp)
void analyzeType(TypeWithStorage type, TypeWithStorage& base, SmallVector<TypeWrapper>& wrappers) {
    wrappers.clear();
    wrappers.reserve(type.numReferences());
    TypeWrapper twPtrDefault = twPtrMutable;
    while (true) {
        TypeWithStorage nextType;
        TypeWrapper tw;
        if (type.kind() == typeKindPtr) {
            tw = twPtrDefault;
            nextType = PtrType(type).base();
            if (nextType.kind() == typeKindConst) {
                tw = twPtrConst;
                nextType = ConstType(nextType).base();
            } else if (nextType.kind() == typeKindMutable) {
                tw = twPtrMutable;
                nextType = MutableType(nextType).base();
            } else if (nextType.kind() == typeKindTemp) {
                tw = twPtrTemp;
                nextType = TempType(nextType).base();
            }
        } else if (type.kind() == typeKindConst) {
            tw = twConst;
            // twPtrDefault = twPtrConst;   // transient category types?
            nextType = ConstType(type).base();
        } else if (type.kind() == typeKindMutable) {
            tw = twMutable;
            // twPtrDefault = twPtrMutable; // transient category types?
            nextType = MutableType(type).base();
        } else if (type.kind() == typeKindTemp) {
            tw = twTemp;
            // twPtrDefault = twPtrTemp;    // transient category types?
            nextType = TempType(type).base();
        } else
            break;

        wrappers.push_back(tw);
        ASSERT(nextType);
        type = nextType;
    }
    std::reverse(wrappers.begin(), wrappers.end());
    base = type;
}

//! Replace the base type from the given type; all the other wrappers remain exactly the same
TypeWithStorage replaceBaseType(TypeWithStorage type, TypeWithStorage newBase) {
    SmallVector<int> kinds;
    kinds.reserve(type.numReferences());

    while (true) {
        auto kind = type.kind();
        if (type.kind() == typeKindPtr) {
            type = PtrType(type).base();
        } else if (type.kind() == typeKindConst) {
            type = ConstType(type).base();
        } else if (type.kind() == typeKindMutable) {
            type = MutableType(type).base();
        } else if (type.kind() == typeKindTemp) {
            type = TempType(type).base();
        } else
            break;
        kinds.push_back(kind);
    }
    TypeWithStorage res = newBase;
    for (auto k : reverse(kinds)) {
        if (k == typeKindPtr)
            res = PtrType::get(res);
        else if (k == typeKindConst)
            res = ConstType::get(res);
        else if (k == typeKindMutable)
            res = MutableType::get(res);
        else if (k == typeKindTemp)
            res = TempType::get(res);
    }
    return res;
}

enum ElemConvType {
    none = 0,  // no conversion possible
    direct,    // same type
    addPtr,    // add pointer
    removePtr, // remove pointer
    catCast,   // cast between categories (with extra refs)
    addCat,    // plain -> category
    removeCat, // category -> plain
    ptr2Cat,   // ptr -> category
    cat2Ptr,   // category -> ptr
};

ostream& operator<<(ostream& os, ElemConvType tw) {
    switch (tw) {
    case none:
        os << "none";
        break;
    case direct:
        os << "direct";
        break;
    case addPtr:
        os << "addPtr";
        break;
    case removePtr:
        os << "removePtr";
        break;
    case catCast:
        os << "catCast";
        break;
    case addCat:
        os << "addCat";
        break;
    case removeCat:
        os << "removeCat";
        break;
    case ptr2Cat:
        os << "ptr2Cat";
        break;
    case cat2Ptr:
        os << "cat2Ptr";
        break;
    }
    return os;
}

//! Check an elementary casts; looks only at the kinds of the top-most types.
ElemConvType checkElementaryCast(TypeWrapper src, TypeWrapper dest) {
    // clang-format off
    constexpr ElemConvType conversions[8][8] = {
        {direct,    addPtr,  addCat,  none,    addCat,  addPtr,  addPtr,  addPtr },// from plain
        {removePtr, direct,  ptr2Cat, ptr2Cat, ptr2Cat, catCast, direct,  none   },// from ptr
        {removeCat, cat2Ptr, direct,  none,    none,    cat2Ptr, none,    none   },// from const
        {removeCat, cat2Ptr, catCast, direct,  none,    cat2Ptr, cat2Ptr, none   },// from mutable
        {removeCat, cat2Ptr, catCast, catCast, direct,  cat2Ptr, cat2Ptr, cat2Ptr},// from temp
        {removePtr, none,    ptr2Cat, none,    none,    direct,  none,    none   },// from ptr const
        {removePtr, direct,  ptr2Cat, ptr2Cat, none,    catCast, direct,  none   },// from ptr mutable
        {removePtr, catCast, catCast, catCast, catCast, catCast, catCast, direct },// from ptr temp
    };
    // to: plain,   ptr,     const,   mutable, temp,    p-const, p-mut,   p-temp
    // clang-format on

    return conversions[src][dest];
}

//! Check cast for cat types wrapped by ptr (both on src and dest)
ElemConvType checkInPtrCast(TypeWrapper src, TypeWrapper dest) {
    ASSERT(isPtr(src));
    ASSERT(isPtr(dest));

    // clang-format off
    constexpr ElemConvType conversions[8][8] = {
        {none, none,    none, none, none, none,    none,    none   },// from plain
        {none, direct,  none, none, none, catCast, direct,  none   },// from ptr
        {none, none,    none, none, none, none,    none,    none   },// from const
        {none, none,    none, none, none, none,    none,    none   },// from mutable
        {none, none,    none, none, none, none,    none,    none   },// from temp
        {none, none,    none, none, none, direct,  none,    none   },// from ptr const
        {none, direct,  none, none, none, catCast, direct,  none   },// from ptr mutable
        {none, catCast, none, none, none, catCast, catCast, direct },// from ptr temp
    };
    // to: plain, ptr,  const,mut,  temp, p-const, p-mut,   p-temp
    // clang-format on
    return conversions[src][dest];
}

//! A stack of node kinds, from which we can pop the base kinds.
struct WrappersStack {
    SmallVector<TypeWrapper> wrappers;
    int cur{0};

    //! Get the index kind at the given index; returns twPlain if going over bounds
    TypeWrapper operator[](int idx) const {
        return cur + idx < wrappers.size() ? wrappers[cur + idx] : twPlain;
    }

    //! Is the stack empty
    bool empty() const { return cur >= wrappers.size(); }

    //! The number of elements remaining in the stack
    int size() const { return int(wrappers.size()) - cur; }

    //! Consume the first kind from the stack
    TypeWrapper pop() {
        assert(!empty());
        auto res = wrappers[cur];
        cur++;
        return res;
    }
};

} // namespace

bool ConvertServiceImpl::checkWrapperTypes(
        ConversionResult& res, TypeWithStorage src, TypeWithStorage dest) {

    // Analyze the two types: figure our their base type and all the wrappers
    WrappersStack srcWrappers;
    WrappersStack destWrappers;
    TypeWithStorage srcBase, destBase;
    analyzeType(src, srcBase, srcWrappers.wrappers);
    analyzeType(dest, destBase, destWrappers.wrappers);

    bool doDebug = false; // StringRef(src.description()) == "i8/ct ptr ptr ptr mut";// &&
                          // StringRef(dest.description()) == "i8 mut ptr";
    if (doDebug) {
        cerr << src << " -> " << dest << "\n";
        cerr << "    src: base=" << srcBase << " wrappers =";
        for (auto w : srcWrappers.wrappers)
            cerr << " " << w;
        cerr << "\n";
        cerr << "    dest: base=" << destBase << " wrappers =";
        for (auto w : destWrappers.wrappers)
            cerr << " " << w;
        cerr << "\n";
    }

    // Handle the case where the destination is a concept
    // Apply the dest shape on the base source type
    if (destBase.kind() == typeKindConcept) {
        dest = replaceBaseType(dest, srcBase);
        destBase = srcBase;
    }

    // Check origin
    if (srcBase != destBase)
        return false;

    // First clear up the pointers from both sides -- advance in tandem
    // We match pointer to pointer
    // We check categories at every iteration
    // Note: between pointers we may have at most one cat type, but nothing else.
    bool needsCast = false;
    bool needsImplicit = false;
    while (!srcWrappers.empty() && !destWrappers.empty()) {
        TypeWrapper srcW = srcWrappers[0];
        TypeWrapper destW = destWrappers[0];

        bool srcIsPtr = isPtr(srcW);
        bool destIsPtr = isPtr(destW);
        if (destIsPtr && !srcIsPtr)
            return false; // cannot add ptr
        if (!destIsPtr || !srcIsPtr)
            break;

        srcWrappers.pop();
        destWrappers.pop();

        // Check elementary casts between possible category types
        auto conv = checkInPtrCast(srcW, destW);
        if (doDebug)
            cerr << "    iter check: " << srcW << " -> " << destW << " = " << conv << "\n";
        if (conv == none)
            return false;

        // TODO (now): Check possible values of ElemConvType
        if (conv != direct) {
            needsCast = true;
            needsImplicit = true;
        }
    }
    // Ensure there are no more pointers on the dest side
    if (!destWrappers.empty() && isPtr(destWrappers[0]))
        return false;

    // Now the middle part, after the common pointers
    // dest may or may not have a cat left
    // Try to consume everything from dest
    bool shouldAddCat = false;
    bool needsDeref = false;
    auto srcW = srcWrappers[0];
    auto destW = destWrappers[0];
    auto conv = checkElementaryCast(srcW, destW);
    if (doDebug)
        cerr << "    top check: " << srcW << " -> " << destW << " = " << conv << "\n";
    switch (conv) {
    case none:
        return false;
    case direct:
        // T <cat> <others>* -> U <cat> (where T and U are ref-equivalent)
        if (!srcWrappers.empty())
            srcWrappers.pop();
        break;
    case catCast:
        // T <cat1> <others>* -> U <cat2> (where T and U are ref-equivalent)
        needsCast = true;
        needsImplicit = true;
        srcWrappers.pop();
        break;
    case addCat:
        // T -> U <cat> (where T and U are ref-equivalent)
        // the source doesn't have anymore ptrs
        if (srcWrappers.cur > 0 && destW == twTemp)
            return false; // Forbid adding 'temp' on ptr types
        ASSERT(srcWrappers.empty());
        needsImplicit = true;
        shouldAddCat = true;
        break;
    case ptr2Cat:
        // T ptr <others>* -> U <cat>
        needsImplicit = true;
        needsCast = true;
        srcWrappers.pop();
        break;
    case removeCat:
        // T <cat> <others>* -> U
        // needsImplicit = true;
        needsDeref = true;
        if (!srcWrappers.empty())
            srcWrappers.pop();
        break;
    case removePtr:
        // T ptr <others>* -> U
        // Don't do anything. Treat this withing general deref
        break;
    case cat2Ptr:
        return false;
    case addPtr:
    default:
        ASSERT(false);
        return false;
    }

    // Dest should be empty now
    if (!destWrappers.empty()) {
        destWrappers.pop();
        ASSERT(destWrappers.empty());
    }

    // Do we just need to add a category?
    if (shouldAddCat) {
        ASSERT(srcWrappers.empty());
        res.addConversion(convDirect, ConvAction(ActionType::addRef, dest));
        return true;
    }

    // We may still have some wrapper types in src.
    // Process them in reverse order now
    int numDerefs = srcWrappers.size() - destWrappers.size();
    if (doDebug)
        cerr << "    num derefs=" << numDerefs << "\n";
    for (int i = 0; i < numDerefs; i++) {
        src = dereferenceType(src);
        res.addConversion(convImplicit, ConvAction(ActionType::dereference, src));
    }
    // Handle cases like T mut -> T or T ptr mut -> T ptr
    if (Feather::isCategoryType(src) && src.numReferences() > dest.numReferences()) {
        src = removeCategoryIfPresent(src);
        res.addConversion(convDirect, ConvAction(ActionType::dereference, src));
    }
    if (doDebug)
        cerr << "    src=" << src << " dest=" << dest << "\n";
    if (src != dest) {
        if (needsCast)
            res.addConversion(needsImplicit ? convImplicit : convDirect,
                    ConvAction(ActionType::bitcast, dest));
        else if (src.numReferences() > dest.numReferences())
            res.addConversion(convImplicit, ConvAction(ActionType::dereference, dest));
        else if (src.numReferences() == dest.numReferences()) {
            res.addConversion(convImplicit, ConvAction(ActionType::bitcast, dest));
        } else
            REP_INTERNAL(NOLOC, "Invalid conversion between %1% and %2%") % src % dest;
    }

    if (doDebug) {
        cerr << "    conv ok:\n";
        for (auto p : res.convertActions())
            cerr << "        " << p.first << " => " << p.second << "\n";
    }
    return true;
}

const ConversionResult& ConvertServiceImpl::cachedCheckConversion(
        CompilationContext* context, int flags, Type srcType, Type destType) {
    PROFILING_ZONE();

    // Try to find the conversion in the map -- first, try without a source code
    KeyType key(srcType, destType, flags, nullptr);
    auto it = conversionMap_.find(key);
    if (it != conversionMap_.end())
        return it->second;
    // Now try with a source code
    key = KeyType(srcType, destType, flags, context->sourceCode);
    it = conversionMap_.find(key);
    if (it != conversionMap_.end())
        return it->second;

    // Compute the value normally
    ConversionResult res = checkConversionImpl(context, flags, srcType, destType);

    // cout << srcType << " -> " << destType << " : " << res << endl;
    // cout << srcType << " -> " << destType << " (" << flags << ") :" << res << endl;

    // Put the result in the cache, if not context dependent
    std::get<3>(key) = res.sourceCode();
    auto r = conversionMap_.insert(make_pair(key, res));

    return r.first->second;
}

} // namespace SprFrontend
