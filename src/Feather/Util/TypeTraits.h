#pragma once

FWD_CLASS1(Nest, Type);
FWD_CLASS1(Nest, Node);
FWD_CLASS1(Feather, Class);

#include <Nest/Intermediate/EvalMode.h>

namespace Feather
{
    /// Tests if the given type/expression is available at CT
    bool isCt(Nest::Type* type);
    bool isCt(Nest::Node* node);
    bool isCt(const vector<Nest::Type*>& types);
    bool isCt(const Nest::NodeVector& nodes);
    
    /// Tests if the given type/expression has a "Testable" type
    bool isTestable(Nest::Type* type);
    bool isTestable(Nest::Node* node);
    
    /// Tests if the given type/expression is an integer type
    bool isInteger(Nest::Type* type);
    bool isInteger(Nest::Node* node);

    /// Checks if the given type is a basic numeric type (bool, integral, floating point, char)
    bool isBasicNumericType(Nest::Type* type);
    
    /// Creates a new type from the original type, and change its mode
    Nest::Type* changeTypeMode(Nest::Type* type, Nest::EvalMode mode, const Nest::Location& loc = Nest::Location());
    
    /// Add a reference to the given type
    Nest::Type* addRef(Nest::Type* type);

    /// Remove a reference to the given type
    Nest::Type* removeRef(Nest::Type* type);

    /// Remove all the references from the given type
    Nest::Type* removeAllRef(Nest::Type* type);

    /// Remove the lvalue from a type - the number of references reported will also decrease
    Nest::Type* removeLValue(Nest::Type* type);
    Nest::Type* removeLValueIfPresent(Nest::Type* type);

    /// Transforms a lvalue into a reference - the reported number of references will remain the same
    Nest::Type* lvalueToRef(Nest::Type* type);
    Nest::Type* lvalueToRefIfPresent(Nest::Type* type);

    /// Gets the class declaration for the given type; if it doesn't have one, return null
    Class* classForType(Nest::Type* t);
    Nest::Node* classForTypeRaw(Nest::Type* t);

    /// Check the types are the same, but also consider the operation mode
    bool isSameTypeIgnoreMode(Nest::Type* t1, Nest::Type* t2);

    // Combines two modes together; raises error if the modes cannot be combined
    Nest::EvalMode combineMode(Nest::EvalMode mode, Nest::EvalMode baseMode, const Nest::Location& loc, bool forceBase = false);

    /// Adjust the mode of the type, to match the evaluation mode of the compilation context
    Nest::Type* adjustMode(Nest::Type* srcType, Nest::CompilationContext* context, const Nest::Location& loc);
    /// Adjust the mode of the type, to match the evaluation mode of the compilation context; takes in account a
    /// 'baseMode' that is the means trough which we can actually access the given type
    Nest::Type* adjustMode(Nest::Type* srcType, Nest::EvalMode baseMode, Nest::CompilationContext* context, const Nest::Location& loc);

    /// Check if the given node has the eval-mode correctly set
    void checkEvalMode(Nest::Node* src, Nest::EvalMode referencedEvalMode = Nest::modeRtCt);
}
