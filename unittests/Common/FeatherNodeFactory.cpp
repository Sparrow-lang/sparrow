#include "StdInc.h"
#include "FeatherNodeFactory.hpp"
#include "LocationGen.hpp"
#include "TypeFactory.hpp"
#include "RcBasic.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"
#include "Nest/Utils/cppif/NodeUtils.hpp"
#include "Nest/Api/Compiler.h"

using namespace rc;
using namespace Feather;

Nest::TypeWithStorage FeatherNodeFactory::genTypeIfNeeded(
        Nest::TypeWithStorage expectedType, bool needsRef) {
    if (expectedType) {
        if (needsRef)
            REQUIRE(expectedType.numReferences() > 0);
        return expectedType;
    } else
        return *TypeFactory::arbBasicStorageType(modeUnspecified, needsRef ? 1 : 0);
}

NodeHandle FeatherNodeFactory::genTypeNode(Nest::TypeWithStorage type) {
    if (*gen::inRange(0, 100) < 80) // 80% using type nodes
        return TypeNode::create(g_LocationGen(), type);
    else
        return *arbExp(type);
}

bool FeatherNodeFactory::hasBoolType() {
    if (testedForBoolType_)
        return hasBoolType_;

    hasBoolType_ = false;
    for (auto decl : TypeFactory::g_dataTypeDecls) {
        if (DeclNode(decl).name() == "i1") {
            hasBoolType_ = true;
            break;
        }
    }
    testedForBoolType_ = true;
    return hasBoolType_;
}

FeatherNodeFactory& FeatherNodeFactory::instance() {
    static FeatherNodeFactory theInstance;
    return theInstance;
}

void FeatherNodeFactory::init() {
    reset();
}

void FeatherNodeFactory::reset() {
    generatedVarDecls_.clear();
    generatedFunDecls_.clear();
    maxDepth_ = 5;
    testedForBoolType_ = false;
    hasBoolType_ = false;
}

void FeatherNodeFactory::clearAuxNodes() {
    generatedVarDecls_.clear();
    generatedFunDecls_.clear();
}
void FeatherNodeFactory::setContextForAuxNodes(Nest::CompilationContext* ctx) {
    for (auto& p : generatedVarDecls_) {
        if (!p.second.context())
            p.second.setContext(ctx);
    }
    for (auto& p : generatedFunDecls_) {
        if (!p.second.context())
            p.second.setContext(ctx);
    }
}

Gen<Nop> FeatherNodeFactory::arbNop() {
    return gen::exec([] { return Nop::create(g_LocationGen()); });
}

Gen<TypeNode> FeatherNodeFactory::arbTypeNode(Nest::TypeWithStorage expectedType) {
    return gen::exec([=] {
        auto t = genTypeIfNeeded(expectedType);
        return TypeNode::create(g_LocationGen(), t);
    });
}

Gen<CtValueExp> FeatherNodeFactory::arbCtValueExp(Nest::TypeWithStorage expectedType) {
    return gen::exec([=] {
        auto t = genTypeIfNeeded(expectedType);
        auto dataSize = int(Nest_sizeOf(t));
        Nest::StringRefM data{dataSize};
        // Fill the data with a char
        char ch = char('0' + *rc::gen::inRange(0, 10));
        for (int i = 0; i < dataSize; i++)
            data.begin[i] = ch;
        return CtValueExp::create(g_LocationGen(), t, data);
    });
}
Gen<Feather::NullExp> FeatherNodeFactory::arbNullExp(Nest::TypeWithStorage expectedType) {
    return gen::exec([=] {
        if (expectedType)
            REQUIRE(expectedType.numReferences() > 0);
        auto t = genTypeIfNeeded(expectedType, true);
        NodeHandle typeNode = genTypeNode(t);
        return NullExp::create(g_LocationGen(), typeNode);
    });
}

Gen<Feather::VarRefExp> FeatherNodeFactory::arbVarRefExp(Nest::TypeWithStorage expectedType) {
    return gen::exec([=] {
        // Get the types needed here
        auto t = expectedType;
        if (expectedType)
            REQUIRE(expectedType.kind() == typeKindMutable);
        else
            t = *TypeFactory::arbMutableType();
        auto baseType = MutableType(t).base();

        // Generate a variable of the base type
        VarDecl& varDecl = generatedVarDecls_[baseType];
        if (!varDecl) {
            string name = string("var-") + baseType.description();
            auto typeNode = TypeNode::create(g_LocationGen(), baseType);
            varDecl = VarDecl::create(g_LocationGen(), name, typeNode);
        }
        // Now make a VarRefExp pointing to the variable
        return VarRefExp::create(g_LocationGen(), varDecl);
    });
}
// Gen<Feather::FieldRefExp> FeatherNodeFactory::arbFieldRefExp(Nest::TypeWithStorage expectedType)
// {
//     return arbCtValueExp(expectedType);
// }
// Gen<Feather::FunRefExp> FeatherNodeFactory::arbFunRefExp(Nest::TypeWithStorage expectedType) {
//     return arbCtValueExp(expectedType);
// }
Gen<Feather::FunCallExp> FeatherNodeFactory::arbFunCallExp(Nest::TypeWithStorage expectedType) {
    return gen::exec([=] {
        // Get the function type
        FunctionType fType;
        if (expectedType) {
            if (expectedType.kind() == Feather::FunctionType::staticKind())
                fType = FunctionType(expectedType);
            else
                fType = *TypeFactory::arbFunctionType(expectedType.mode(), expectedType);
        } else
            fType = *TypeFactory::arbFunctionType(modeRt);

        // Generate a function (if we don't already have one for the same type)
        auto loc = g_LocationGen();
        FunctionDecl& funDecl = generatedFunDecls_[fType];
        if (!funDecl) {
            vector<Nest::Node*> params;
            params.reserve(fType.numParams());
            for (int i = 0; i < fType.numParams(); i++) {
                params.push_back(VarDecl::create(loc, "p", TypeNode::create(loc, fType[i])));
            }
            auto retType = TypeNode::create(loc, fType.result());
            funDecl = FunctionDecl::create(loc, "f", retType, Nest::all(params), {});

            // Mark some functions as 'autoCt'
            if (!expectedType && *gen::inRange(0, 4) == 3)
                funDecl.setProperty(propAutoCt, 1);
        }

        // Generate the arguments
        maxDepth_--;
        vector<Nest::Node*> args;
        args.reserve(fType.numParams());
        for (int i = 0; i < fType.numParams(); i++)
            args.push_back(*arbExp(fType[i]));
        maxDepth_++;

        return FunCallExp::create(g_LocationGen(), funDecl, Nest::all(args));
    });
}
Gen<Feather::MemLoadExp> FeatherNodeFactory::arbMemLoadExp(Nest::TypeWithStorage expectedType) {
    return gen::exec([=] {
        auto t = expectedType;
        if (expectedType) {
            t = addRef(expectedType);
            REQUIRE(t.numReferences() > 0);
        } else
            t = genTypeIfNeeded({}, true);
        REQUIRE(t.numReferences() > 0);
        NodeHandle address = *arbExp(t);
        return MemLoadExp::create(g_LocationGen(), address);
    });
}
Gen<Feather::MemStoreExp> FeatherNodeFactory::arbMemStoreExp() {
    return gen::exec([=] {
        auto tVal = genTypeIfNeeded({}, true);
        NodeHandle value = *arbExp(tVal);
        NodeHandle address = *arbExp(addRef(tVal));
        return MemStoreExp::create(g_LocationGen(), value, address);
    });
}
Gen<Feather::BitcastExp> FeatherNodeFactory::arbBitcastExp(Nest::TypeWithStorage expectedType) {
    return gen::exec([=] {
        if (expectedType)
            REQUIRE(expectedType.numReferences() > 0);
        auto tDest = genTypeIfNeeded(expectedType, true);
        auto tSrc = genTypeIfNeeded({}, true);
        NodeHandle typeNode = genTypeNode(tDest);
        NodeHandle exp = *arbExp(tSrc);
        return BitcastExp::create(g_LocationGen(), typeNode, exp);
    });
}
Gen<Feather::ConditionalExp> FeatherNodeFactory::arbConditionalExp(
        Nest::TypeWithStorage expectedType) {
    return gen::exec([=] {
        auto tDest = genTypeIfNeeded(expectedType);
        auto condMode = tDest.mode() == modeCt ? modeCt : modeUnspecified;
        auto tCond = *TypeFactory::arbBoolType(condMode);
        NodeHandle cond = *arbExp(tCond);
        NodeHandle alt1 = *arbExp(tDest);
        NodeHandle alt2 = *arbExp(tDest);
        return ConditionalExp::create(g_LocationGen(), cond, alt1, alt2);
    });
}

Gen<NodeHandle> FeatherNodeFactory::arbExp(Nest::TypeWithStorage expectedType) {
    int weightCtValueExp = 3;
    int weightNullExp = 2;
    int weightVarRefExp = 2;
    // int weightFieldRefExp = 2;
    // int weightFunRefExp = 1;
    int weightFunCallExp = 10;
    int weightMemLoadExp = 5;
    int weightBitcastExp = 3;
    int weightConditionalExp = 1;

    if (expectedType) {
        auto kind = expectedType.kind();
        if (kind != Feather::DataType::staticKind()) {
            weightMemLoadExp = 0;
            weightConditionalExp = 0;
        }
        if (kind != typeKindMutable) {
            weightVarRefExp = 0;
            // weightFieldRefExp = 0;
            // weightFunRefExp = 0;
        }
        if (expectedType.numReferences() == 0) {
            weightNullExp = 0;
            weightVarRefExp = 0;
            // weightFieldRefExp = 0;
            // weightFunRefExp = 0;
            weightBitcastExp = 0;
        } else {
            weightCtValueExp = 0;
        }
    }
    if (maxDepth_ <= 0)
        weightFunCallExp = 0;
    if (!hasBoolType())
        weightConditionalExp = 0;

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
