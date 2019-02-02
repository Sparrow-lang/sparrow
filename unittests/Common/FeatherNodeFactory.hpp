#pragma once

#include "Feather/Utils/cppif/FeatherNodes.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

class FeatherNodeFactory {
public:
    static FeatherNodeFactory& instance();

    //! Initialize the context for a test
    void init();
    //! Reset all the context data
    void reset();
    //! Remove all the aux generated decls
    void clearAuxNodes();
    //! Set the context for all the aux generated decls
    void setContextForAuxNodes(Nest::CompilationContext* ctx);

    rc::Gen<Feather::Nop> arbNop();
    rc::Gen<Feather::TypeNode> arbTypeNode(Nest::TypeWithStorage expectedType = {});

    rc::Gen<Feather::CtValueExp> arbCtValueExp(Nest::TypeWithStorage expectedType = {});
    rc::Gen<Feather::NullExp> arbNullExp(Nest::TypeWithStorage expectedType = {});
    rc::Gen<Feather::VarRefExp> arbVarRefExp(Nest::TypeWithStorage expectedType = {});
    rc::Gen<Feather::FieldRefExp> arbFieldRefExp(Nest::TypeWithStorage expectedType = {});
    rc::Gen<Feather::FunRefExp> arbFunRefExp(Nest::TypeWithStorage expectedType = {});
    rc::Gen<Feather::FunCallExp> arbFunCallExp(Nest::TypeWithStorage expectedType = {});
    rc::Gen<Feather::MemLoadExp> arbMemLoadExp(Nest::TypeWithStorage expectedType = {});
    rc::Gen<Feather::MemStoreExp> arbMemStoreExp();
    rc::Gen<Feather::BitcastExp> arbBitcastExp(Nest::TypeWithStorage expectedType = {});
    rc::Gen<Feather::ConditionalExp> arbConditionalExp(Nest::TypeWithStorage expectedType = {});

    rc::Gen<Nest::NodeHandle> arbExp(Nest::TypeWithStorage expectedType = {});

private:
    //! Generate a type of type, if one 'expectedType' is empty
    //! To be called from generators
    Nest::TypeWithStorage genTypeIfNeeded(
            Nest::TypeWithStorage expectedType, bool needsRef = false);

    //! Generate a node that can be used as a type node
    //! To be called from generators
    Nest::NodeHandle genTypeNode(Nest::TypeWithStorage type);

    //! Checks if we have a bool type
    bool hasBoolType();

private:
    //! The list of generated var decls, as auxiliary data
    unordered_map<Nest::TypeRef, Feather::VarDecl> generatedVarDecls_;
    //! The list of generated function decls, as auxiliary data
    unordered_map<Nest::TypeRef, Feather::FunctionDecl> generatedFunDecls_;
    //! The maximum depth of nodes we would like to build -- avoiding infinite recursion
    int maxDepth_{5};
    //! Indicates if we checked whether we have a bool type
    bool testedForBoolType_;
    //! True if we have a bool type
    bool hasBoolType_;
};

namespace rc {

    template <> struct Arbitrary<Feather::Nop> {
        static Gen<Feather::Nop> arbitrary() { return FeatherNodeFactory::instance().arbNop(); }
    };
    template <> struct Arbitrary<Feather::TypeNode> {
        static Gen<Feather::TypeNode> arbitrary() {
            return FeatherNodeFactory::instance().arbTypeNode();
        }
    };
    template <> struct Arbitrary<Feather::CtValueExp> {
        static Gen<Feather::CtValueExp> arbitrary() {
            return FeatherNodeFactory::instance().arbCtValueExp();
        }
    };
    template <> struct Arbitrary<Feather::NullExp> {
        static Gen<Feather::NullExp> arbitrary() {
            return FeatherNodeFactory::instance().arbNullExp();
        }
    };
    template <> struct Arbitrary<Feather::VarRefExp> {
        static Gen<Feather::VarRefExp> arbitrary() {
            return FeatherNodeFactory::instance().arbVarRefExp();
        }
    };
    template <> struct Arbitrary<Feather::FieldRefExp> {
        static Gen<Feather::FieldRefExp> arbitrary() {
            return FeatherNodeFactory::instance().arbFieldRefExp();
        }
    };
    template <> struct Arbitrary<Feather::FunRefExp> {
        static Gen<Feather::FunRefExp> arbitrary() {
            return FeatherNodeFactory::instance().arbFunRefExp();
        }
    };
    template <> struct Arbitrary<Feather::FunCallExp> {
        static Gen<Feather::FunCallExp> arbitrary() {
            return FeatherNodeFactory::instance().arbFunCallExp();
        }
    };
    template <> struct Arbitrary<Feather::MemLoadExp> {
        static Gen<Feather::MemLoadExp> arbitrary() {
            return FeatherNodeFactory::instance().arbMemLoadExp();
        }
    };
    template <> struct Arbitrary<Feather::MemStoreExp> {
        static Gen<Feather::MemStoreExp> arbitrary() {
            return FeatherNodeFactory::instance().arbMemStoreExp();
        }
    };
    template <> struct Arbitrary<Feather::BitcastExp> {
        static Gen<Feather::BitcastExp> arbitrary() {
            return FeatherNodeFactory::instance().arbBitcastExp();
        }
    };
    template <> struct Arbitrary<Feather::ConditionalExp> {
        static Gen<Feather::ConditionalExp> arbitrary() {
            return FeatherNodeFactory::instance().arbConditionalExp();
        }
    };

} // namespace rc
