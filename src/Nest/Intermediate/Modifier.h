#pragma once

typedef struct Nest_Node Node;

enum Nest_ModifierType {
    modTypeBeforeSetContext,
    modTypeAfterSetContext,
    modTypeBeforeComputeType,
    modTypeAfterComputeType,
    modTypeBeforeSemanticCheck,
    modTypeAfterSemanticCheck,
};
typedef enum Nest_ModifierType Nest_ModifierType;
typedef enum Nest_ModifierType ModifierType;

typedef struct Nest_Modifier Modifier;
typedef struct Nest_Modifier Nest_Modifier;

typedef void (*FModifierFun)(Nest_Modifier* mod, Node* node);

/// Structure defining an interface for a modifier
/// The modifier is called for all the important operations that happen to a node
/// This way, the modifier can change the compilation of the node
struct Nest_Modifier {
    ModifierType modifierType;
    FModifierFun modifierFun;
};

struct Nest_ModifiersArray {
    Modifier** beginPtr;
    Modifier** endPtr;
    Modifier** endOfStorePtr;
};
typedef struct Nest_ModifiersArray Nest_ModifiersArray;
typedef struct Nest_ModifiersArray ModifiersArray;
