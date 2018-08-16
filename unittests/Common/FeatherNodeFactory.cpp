#include "StdInc.h"
#include "FeatherNodeFactory.hpp"
#include "TypeFactory.hpp"
#include "RcBasic.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"
#include "Nest/Utils/cppif/NodeUtils.hpp"
#include "Nest/Api/Compiler.h"

using namespace rc;
using namespace Feather;

Feather::TypeWithStorage FeatherNodeFactory::genTypeIfNeeded(
        Feather::TypeWithStorage expectedType, bool needsRef) {
    if (expectedType) {
        if (needsRef)
            REQUIRE(expectedType.numReferences() > 0);
        return expectedType;
    } else
        return *TypeFactory::arbBasicStorageType(modeUnspecified, needsRef ? 1 : 0);
}

NodeHandle FeatherNodeFactory::genTypeNode(Feather::TypeWithStorage type) {
    if (*gen::inRange(0, 100) < 80) // 80% using type nodes
        return TypeNode::create(locationGen(), type);
    else
        return *arbExp(type);
}

FeatherNodeFactory& FeatherNodeFactory::instance() {
    static FeatherNodeFactory theInstance;
    return theInstance;
}

void FeatherNodeFactory::init(LocationGenFun locGen) {
    reset();
    locationGen = locGen;
}

void FeatherNodeFactory::reset() {
    generatedVarDecls_.clear();
    generatedFunDecls_.clear();
    maxDepth_ = 5;
    locationGen = {};
}

void FeatherNodeFactory::setContextForAuxNodes(Nest::CompilationContext* ctx) {
    for (auto& p : generatedVarDecls_) {
        p.second.setContext(ctx);
    }
    for (auto& p : generatedFunDecls_) {
        p.second.setContext(ctx);
    }
}


Gen<Nop> FeatherNodeFactory::arbNop() {
    return gen::exec([this] { return Nop::create(locationGen()); });
}

Gen<TypeNode> FeatherNodeFactory::arbTypeNode() {
    return gen::exec(
            [this] { return TypeNode::create(locationGen(), *TypeFactory::arbBasicStorageType()); });
}

Gen<CtValueExp> FeatherNodeFactory::arbCtValueExp(Feather::TypeWithStorage expectedType) {
    return gen::exec([=] {
        auto t = genTypeIfNeeded(expectedType);
        Nest::StringRefM data{int(Nest_sizeOf(t))};
        // Leave the data to be random
        return CtValueExp::create(locationGen(), t, data);
    });
}
Gen<Feather::NullExp> FeatherNodeFactory::arbNullExp(Feather::TypeWithStorage expectedType) {
    return gen::exec([=] {
        if (expectedType)
            REQUIRE(expectedType.numReferences() > 0);
        auto t = genTypeIfNeeded(expectedType, true);
        NodeHandle typeNode = genTypeNode(t);
        return NullExp::create(locationGen(), typeNode);
    });
}

Gen<Feather::VarRefExp> FeatherNodeFactory::arbVarRefExp(Feather::TypeWithStorage expectedType) {
    return gen::exec([=] {
        // Get the types needed here
        auto t = expectedType;
        if (expectedType)
            REQUIRE(expectedType.kind() == Feather_getLValueTypeKind());
        else
            t = *TypeFactory::arbLValueType();
        auto baseType = LValueType(t).base();

        // Generate a variable of the base type
        VarDecl& varDecl = generatedVarDecls_[baseType];
        if (!varDecl) {
            string name = string("var-") + baseType.description();
            varDecl = VarDecl::create(locationGen(), name, genTypeNode(baseType));
        }
        // Now make a VarRefExp pointing to the variable
        return VarRefExp::create(locationGen(), varDecl);
    });
}
// Gen<Feather::FieldRefExp> FeatherNodeFactory::arbFieldRefExp(Feather::TypeWithStorage expectedType) {
//     return arbCtValueExp(expectedType);
// }
// Gen<Feather::FunRefExp> FeatherNodeFactory::arbFunRefExp(Feather::TypeWithStorage expectedType) {
//     return arbCtValueExp(expectedType);
// }
Gen<Feather::FunCallExp> FeatherNodeFactory::arbFunCallExp(Feather::TypeWithStorage expectedType) {
    return gen::exec([=] {
        // Get the function type
        FunctionType fType;
        if (expectedType) {
            if (expectedType.kind() == Feather_getFunctionTypeKind())
                fType = FunctionType(expectedType);
            else
                fType = *TypeFactory::arbFunctionType(expectedType.mode(), expectedType);
        } else
            fType = *TypeFactory::arbFunctionType(modeRt);

        // Generate a function (if we don't already have one for the same type)
        auto loc = locationGen();
        FunctionDecl& funDecl = generatedFunDecls_[fType];
        if (!funDecl) {
            vector<Nest::Node*> params;
            params.reserve(fType.numParams());
            for (int i=0; i<fType.numParams(); i++) {
                params.push_back(VarDecl::create(loc, "p", TypeNode::create(loc, fType[i])));
            }
            auto retType = TypeNode::create(loc, fType.result());
            funDecl = FunctionDecl::create(loc, "f", retType, Nest::all(params), {});

            // Mark some functions as 'autoCt'
            if (*gen::inRange(0, 4) == 3)
                funDecl.setProperty(propAutoCt, 1);
        }

        // Generate the arguments
        maxDepth_--;
        vector<Nest::Node*> args;
        args.reserve(fType.numParams());
        for (int i=0; i<fType.numParams(); i++)
            args.push_back(*arbExp(fType[i]));
        maxDepth_++;

        return FunCallExp::create(locationGen(), funDecl, Nest::all(args));
    });
}
Gen<Feather::MemLoadExp> FeatherNodeFactory::arbMemLoadExp(Feather::TypeWithStorage expectedType) {
    return gen::exec([=] {
        auto t = expectedType;
        if (expectedType) {
            t = addRef(expectedType);
            REQUIRE(t.numReferences() > 0);
        }
        else
            t = genTypeIfNeeded({}, true);
        REQUIRE(t.numReferences() > 0);
        NodeHandle address = *arbExp(t);
        return MemLoadExp::create(locationGen(), address);
    });
}
Gen<Feather::MemStoreExp> FeatherNodeFactory::arbMemStoreExp() {
    return gen::exec([=] {
        auto tVal = genTypeIfNeeded({}, true);
        NodeHandle value = *arbExp(tVal);
        NodeHandle address = *arbExp(addRef(tVal));
        return MemStoreExp::create(locationGen(), value, address);
    });
}
Gen<Feather::BitcastExp> FeatherNodeFactory::arbBitcastExp(Feather::TypeWithStorage expectedType) {
    return gen::exec([=] {
        if (expectedType)
            REQUIRE(expectedType.numReferences() > 0);
        auto tDest = genTypeIfNeeded(expectedType, true);
        auto tSrc = genTypeIfNeeded({}, true);
        NodeHandle typeNode = genTypeNode(tDest);
        NodeHandle exp = *arbExp(tSrc);
        return BitcastExp::create(locationGen(), typeNode, exp);
    });
}
Gen<Feather::ConditionalExp> FeatherNodeFactory::arbConditionalExp(Feather::TypeWithStorage expectedType) {
    return gen::exec([=] {
        auto tDest = genTypeIfNeeded(expectedType);
        auto condMode = tDest.mode() == modeCt ? modeCt : modeUnspecified;
        auto tCond = *TypeFactory::arbBoolType(condMode);
        NodeHandle cond = *arbExp(tCond);
        NodeHandle alt1 = *arbExp(tDest);
        NodeHandle alt2 = *arbExp(tDest);
        return ConditionalExp::create(locationGen(), cond, alt1, alt2);
    });
}

Gen<NodeHandle> FeatherNodeFactory::arbExp(Feather::TypeWithStorage expectedType) {
    int weightCtValueExp = 3;
    int weightNullExp = 2;
    int weightVarRefExp = 2;
    int weightFieldRefExp = 2;
    int weightFunRefExp = 1;
    int weightFunCallExp = 10;
    int weightMemLoadExp = 5;
    int weightBitcastExp = 3;
    int weightConditionalExp = 1;

    if (expectedType)
    {
        auto kind = expectedType.kind();
        if (kind != Feather_getDataTypeKind()) {
            weightMemLoadExp = 0;
            weightConditionalExp = 0;
        }
        if (kind != Feather_getLValueTypeKind()) {
            weightVarRefExp = 0;
            weightFieldRefExp = 0;
            weightFunRefExp = 0;
        }
        if ( expectedType.numReferences() == 0) {
            weightNullExp = 0;
            weightVarRefExp = 0;
            weightFieldRefExp = 0;
            weightFunRefExp = 0;
            weightBitcastExp = 0;
        }
    }
    if ( maxDepth_ <= 0 )
        weightFunCallExp = 0;

    return gen::weightedOneOf<NodeHandle>({
            {weightCtValueExp, gen::cast<NodeHandle>(arbCtValueExp(expectedType))},
            {weightNullExp, gen::cast<NodeHandle>(arbNullExp(expectedType))},
            {weightVarRefExp, gen::cast<NodeHandle>(arbVarRefExp(expectedType))},
            // {weightFieldRefExp, gen::cast<NodeHandle>(arbFieldRefExp(expectedType))},
            // {weightFunRefExp, gen::cast<NodeHandle>(arbFunRefExp(expectedType))},
            {weightFunCallExp, gen::cast<NodeHandle>(arbFunCallExp(expectedType))},
            {weightMemLoadExp, gen::cast<NodeHandle>(arbMemLoadExp(expectedType))},
            {weightBitcastExp, gen::cast<NodeHandle>(arbBitcastExp(expectedType))},
            {weightConditionalExp, gen::cast<NodeHandle>(arbConditionalExp(expectedType))},
    });
}
