#pragma once

#include <Nest/Intermediate/NodeVector.h>
#include <Nest/Intermediate/TypeRef.h>
#include <Nest/Intermediate/EvalMode.h>

typedef struct Nest_CompilationContext CompilationContext;

namespace Feather
{
    using Nest::Node;

    /// Tests if the given type/expression is available at CT
    bool isCt(TypeRef type);
    bool isCt(Node* node);
    bool isCt(const vector<TypeRef>& types);
    bool isCt(const Nest::NodeVector& nodes);
    
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
