#pragma once

#include "Feather/Utils/FeatherUtils.h"

#include "Nest/Api/EvalMode.h"
#include "Nest/Api/TypeRef.h"
#include "Nest/Api/StringRef.h"
#include "Nest/Api/Location.h"

typedef struct Nest_Node Node;
typedef struct Nest_CompilationContext CompilationContext;

namespace Feather
{


////////////////////////////////////////////////////////////////////////////////
// Ct

    /// Getter for the value type of a CtValue node
    TypeRef getCtValueType(Node* ctVal);

    /// Getter for the value data of a CtValue node -- the data is encoded in a string
    StringRef getCtValueDataAsString(Node* ctVal);

    /// Getter for the value memory buffer of this value
    template <typename T>
    T* getCtValueData(Node* ctVal)
    {
        return (T*) (void*) getCtValueDataAsString(ctVal).begin;
    }

    bool getBoolCtValue(Node* ctVal);

    /// Check if the two given CtValue objects are equal -- contains the same type and data
    bool sameCtValue(Node* ctVal1, Node* ctVal2);

////////////////////////////////////////////////////////////////////////////////
// Decl

    /// Tests if the given node is a declaration (a node that will expand to a Feather declaration)
    bool isDecl(Node* node);
    /// Tests if the given node is a Feather declaration or has a resuling declaration
    bool isDeclEx(Node* node);
    
    /// Get the name of the given declaration
    /// Throws if the name was not set for the node
    StringRef getName(const Node* decl);

    /// Indicates if the node has associated a name with it
    bool hasName(const Node* decl);

    /// Setter for the name of a declaration
    void setName(Node* decl, StringRef name);
    
    /// Getter for the evaluation mode set in the given declaration (non-ct, ct, auto-ct)
    EvalMode nodeEvalMode(const Node* decl);
    /// Getter for the evaluation mode set in the given declaration (non-ct, ct, auto-ct)
    /// If the given declaration doesn't have an evaluation mode, the evaluation mode of the context is used
    EvalMode effectiveEvalMode(const Node* decl);
    
    /// Setter for the evaluation mode of the given declaration
    void setEvalMode(Node* decl, EvalMode val);

    /// Add the given declaration to the sym tab
    void addToSymTab(Node* decl);

    /// Should we add the given declaration to the symbols table?
    void setShouldAddToSymTab(Node* decl, bool val);

////////////////////////////////////////////////////////////////////////////////
// TypeTraits

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

template <typename T>
Node* Feather_mkCtValueT(const Location& loc, TypeRef type, T* dataVal)
{
    const char* p = reinterpret_cast<const char*>(dataVal);
    StringRef dataStr = {p, p+sizeof(*dataVal)};
    return Feather_mkCtValue(loc, type, dataStr);
}


