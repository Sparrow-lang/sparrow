#pragma once

FWD_STRUCT1(Nest, Node);

typedef struct Nest_Modifier Modifier;

typedef void (*FModifierBeforeSetContext)(Nest_Modifier* mod, Nest::Node* node);
typedef void (*FModifierAfterSetContext)(Nest_Modifier* mod, Nest::Node* node);
typedef void (*FModifierBeforeComputeType)(Nest_Modifier* mod, Nest::Node* node);
typedef void (*FModifierAfterComputeType)(Nest_Modifier* mod, Nest::Node* node);
typedef void (*FModifierBeforeSemanticCheck)(Nest_Modifier* mod, Nest::Node* node);
typedef void (*FModifierAfterSemanticCheck)(Nest_Modifier* mod, Nest::Node* node);

/// Structure defining an interface for a modifier
/// The modifier is called for all the important operations that happen to a node
/// This way, the modifier can change the compilation of the node
struct Nest_Modifier {
    FModifierBeforeSetContext beforeSetContext;
    FModifierAfterSetContext afterSetContext;
    FModifierBeforeComputeType beforeComputeType;
    FModifierAfterComputeType afterComputeType;
    FModifierBeforeSemanticCheck beforeSemanticCheck;
    FModifierAfterSemanticCheck afterSemanticCheck;
};
typedef struct Nest_Modifier Nest_Modifier;
