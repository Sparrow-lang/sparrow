#pragma once

#include "Nest/Utils/NodeVector.hpp"
#include "Nest/Api/TypeRef.h"
#include "Nest/Api/EvalMode.h"

#include <vector>

typedef struct Nest_CompilationContext CompilationContext;

namespace Feather
{
    /// Tests if the given type/expression is available at CT
    bool isCt(TypeRef type);
    bool isCt(Node* node);
    bool isCt(const vector<TypeRef>& types);
    bool isCt(NodeRange nodes);
    
    /// Tests if the given type/expression has a "Testable" type
    bool isTestable(TypeRef type);
    bool isTestable(Node* node);
    
    /// Tests if the given type/expression is an integer type
    bool isInteger(TypeRef type);
    bool isInteger(Node* node);

    /// Checks if the given type is a basic numeric type (bool, integral, floating point, char)
    bool isBasicNumericType(TypeRef type);
    
    /// Creates a new type from the original type, and change its mode
    TypeRef changeTypeMode(TypeRef type, EvalMode mode, const Location& loc = Location());
    
    /// Add a reference to the given type
    TypeRef addRef(TypeRef type);

    /// Remove a reference to the given type
    TypeRef removeRef(TypeRef type);

    /// Remove all the references from the given type
    TypeRef removeAllRef(TypeRef type);

    /// Remove the lvalue from a type - the number of references reported will also decrease
    TypeRef removeLValue(TypeRef type);
    TypeRef removeLValueIfPresent(TypeRef type);

    /// Transforms a lvalue into a reference - the reported number of references will remain the same
    TypeRef lvalueToRef(TypeRef type);
    TypeRef lvalueToRefIfPresent(TypeRef type);

    /// Gets the class declaration for the given type; if it doesn't have one, return null
    Node* classForType(TypeRef t);

    /// Check the types are the same, but also consider the operation mode
    bool isSameTypeIgnoreMode(TypeRef t1, TypeRef t2);

    // Combines two modes together; raises error if the modes cannot be combined
    EvalMode combineMode(EvalMode mode, EvalMode baseMode, const Location& loc, bool forceBase = false);

    /// Adjust the mode of the type, to match the evaluation mode of the compilation context
    TypeRef adjustMode(TypeRef srcType, CompilationContext* context, const Location& loc);
    /// Adjust the mode of the type, to match the evaluation mode of the compilation context; takes in account a
    /// 'baseMode' that is the means trough which we can actually access the given type
    TypeRef adjustMode(TypeRef srcType, EvalMode baseMode, CompilationContext* context, const Location& loc);

    /// Check if the given node has the eval-mode correctly set
    void checkEvalMode(Node* src, EvalMode referencedEvalMode = modeRtCt);
}


#ifdef __cplusplus
extern "C" {
#endif


/// Getter for the class that introduces this data type - can be null
/// Works for only for storage types (data, l-value, array)
Node* Feather_classDecl(TypeRef type);

/// If the class associated with the given type has an associated name this will return it; otherwise it returns nullptr
/// Works for only for storage types (data, l-value, array)
const StringRef* Feather_nativeName(TypeRef type);

/// The number of references applied
/// Works for only for storage types (data, l-value, array)
int Feather_numReferences(TypeRef type);

/// Returns the base type of this type
/// Works for l-value and array types
TypeRef Feather_baseType(TypeRef type);

/// Getter for the number of units in the buffer type
/// Works only for array types
int Feather_getArraySize(TypeRef type);

size_t Feather_numFunParameters(TypeRef type);
TypeRef Feather_getFunParameter(TypeRef type, size_t idx);
// vector<TypeRef> Feather_getFunParameters(TypeRef type);
TypeRef Feather_getFunResultType(TypeRef type);


#ifdef __cplusplus
}
#endif


