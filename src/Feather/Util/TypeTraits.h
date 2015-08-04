#pragma once

FWD_CLASS1(Feather, DynNode);

#include <Nest/Intermediate/TypeRef.h>
#include <Nest/Intermediate/EvalMode.h>

namespace Feather
{
    using Nest::Node;

    /// Tests if the given type/expression is available at CT
    bool isCt(Nest::TypeRef type);
    bool isCt(Node* node);
    bool isCt(const vector<Nest::TypeRef>& types);
    bool isCt(const Nest::NodeVector& nodes);
    
    /// Tests if the given type/expression has a "Testable" type
    bool isTestable(Nest::TypeRef type);
    bool isTestable(Node* node);
    
    /// Tests if the given type/expression is an integer type
    bool isInteger(Nest::TypeRef type);
    bool isInteger(Node* node);

    /// Checks if the given type is a basic numeric type (bool, integral, floating point, char)
    bool isBasicNumericType(Nest::TypeRef type);
    
    /// Creates a new type from the original type, and change its mode
    Nest::TypeRef changeTypeMode(Nest::TypeRef type, Nest::EvalMode mode, const Nest::Location& loc = Nest::Location());
    
    /// Add a reference to the given type
    Nest::TypeRef addRef(Nest::TypeRef type);

    /// Remove a reference to the given type
    Nest::TypeRef removeRef(Nest::TypeRef type);

    /// Remove all the references from the given type
    Nest::TypeRef removeAllRef(Nest::TypeRef type);

    /// Remove the lvalue from a type - the number of references reported will also decrease
    Nest::TypeRef removeLValue(Nest::TypeRef type);
    Nest::TypeRef removeLValueIfPresent(Nest::TypeRef type);

    /// Transforms a lvalue into a reference - the reported number of references will remain the same
    Nest::TypeRef lvalueToRef(Nest::TypeRef type);
    Nest::TypeRef lvalueToRefIfPresent(Nest::TypeRef type);

    /// Gets the class declaration for the given type; if it doesn't have one, return null
    Node* classForType(Nest::TypeRef t);

    /// Check the types are the same, but also consider the operation mode
    bool isSameTypeIgnoreMode(Nest::TypeRef t1, Nest::TypeRef t2);

    // Combines two modes together; raises error if the modes cannot be combined
    Nest::EvalMode combineMode(Nest::EvalMode mode, Nest::EvalMode baseMode, const Nest::Location& loc, bool forceBase = false);

    /// Adjust the mode of the type, to match the evaluation mode of the compilation context
    Nest::TypeRef adjustMode(Nest::TypeRef srcType, Nest::CompilationContext* context, const Nest::Location& loc);
    /// Adjust the mode of the type, to match the evaluation mode of the compilation context; takes in account a
    /// 'baseMode' that is the means trough which we can actually access the given type
    Nest::TypeRef adjustMode(Nest::TypeRef srcType, Nest::EvalMode baseMode, Nest::CompilationContext* context, const Nest::Location& loc);

    /// Check if the given node has the eval-mode correctly set
    void checkEvalMode(Node* src, Nest::EvalMode referencedEvalMode = Nest::modeRtCt);
}
