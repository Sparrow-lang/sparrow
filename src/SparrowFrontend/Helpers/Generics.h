#pragma once

namespace SprFrontend
{
    bool conceptIsFulfilled(Node* concept, TypeRef type);
    bool typeGeneratedFromGeneric(Node* genericClass, TypeRef type);

    TypeRef baseConceptType(Node* concept);

    /// Checks if the given declaration with the given parameters is a generic; if yes, creates an object of this type
    Node* createGenericFun(Node* originalFun, Node* parameters, Node* ifClause, Node* thisClass = nullptr);

    bool isGeneric(const Node* node);
    size_t genericParamsCount(const Node* node);
    Node* genericParam(const Node* node, size_t idx);
    Node* genericCanInstantiate(Node* node, NodeRange args);
    Node* genericDoInstantiate(Node* node, const Location& loc, CompilationContext* context, NodeRange args, Node* instantiation);
}
