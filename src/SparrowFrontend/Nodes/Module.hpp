#pragma once

#include "SparrowFrontend/SparrowFrontendTypes.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"
#include "Nest/Utils/cppif/NodeHelpers.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"

namespace SprFrontend {

using Feather::NodeList;
using Nest::Location;
using Nest::Node;
using Nest::NodeHandle;
using Nest::NodeRange;
using Nest::StringRef;
using Nest::Type;
using Nest::TypeWithStorage;

/**
 * @brief      A Sparrow module node
 *
 * This is created for every Sparrow source-code we have.
 * For a module with name A.B.C and <decls> declarations, we would create the following content:
 *     package A
 *         package B
 *             package C
 *                 import implicitLib;      // if not inside implicitLib
 *                 import compilerDefines;  // if not inside compiler defines module
 *                 <decls>
 *
 * Preconditions:
 *      - module name, if present, is a qualified ID (i.e., A.B.C)
 *
 * Postconditions:
 *     - if module name is not given, we take the name of the filename as module name
 *     - a package will be created for each part of the module name
 *     - the given decls are added to the inner most created package
 *     - if outside implicit lib, an import to the implicit lib is added
 *     - if outside of compiler defines module, an import to that module is added
 *     - a CompilationContext is created for the module
 */
struct Module : Feather::DeclNode {
    DECLARE_NODE_COMMON(Module)

    /**
     * @brief      Creates a Sparrow module node
     *
     * @param[in]  loc        The location of the node
     * @param[in]  moduleName The name of the module -- a qualified identifier
     * @param[in]  decls      The declarations of the module
     *
     * @return     The desired node
     */
    static Module create(const Location& loc, NodeHandle moduleName, NodeList decls);

    //! Returns the node containing the module name
    NodeHandle moduleName() const { return children()[0]; };

    //! Returns the node with the declarations of the module
    NodeList decls() const { return NodeList(children()[1]); };

private:
    static void setContextForChildrenImpl(Module node);
    static NodeHandle semanticCheckImpl(Module node);
};

/**
 * @brief      An import declaration
 *
 * This introduces the decls of the indicated module in the current module.
 * For something like:
 *      import modName(f, g);
 * we return something like:
 *      {
 *          using f = <moduleRef>.f;
 *          using g = <moduleRef>.g;
 *      }
 *
 * For something like:
 *      import modName;
 * we return something like:
 *      using <moduleRef>.*;
 *
 * If alias is given, places everything in a package
 *
 * Preconditions:
 *     - the given module must be able to be loaded
 *     - if decl names are given, the indicated modules must have top-level decls with those names
 *
 * Postconditions:
 *     - using decls will be created to add the decls from the indicated module in the current mod
 *     - the indicated modules can be used in the current module, as if they were declared here
 *     - if alias is given, a package with the alias name will surround the decls
 *     - we can import Sparrow and non-Sparrow modules
 */
struct ImportName : Feather::DeclNode {
    DECLARE_NODE_COMMON(ImportName)

    /**
     * @brief      Creates an import name node
     *
     * @param[in]  loc               The location of the node
     * @param[in]  moduleName        The name of the module to be imported; QID or string literal
     * @param[in]  importedDeclNames The list of decl names to be imported
     * @param[in]  alias             The alias given to the imported content
     *
     * @return     The desired node
     */
    static ImportName create(const Location& loc, NodeHandle moduleName, NodeList importedDeclNames,
            StringRef alias = {});

    //! Returns the node containing the module name we are referring to
    NodeHandle moduleName() const { return children()[0]; };

    //! Returns the imported decls names
    NodeList importedDeclNames() const { return NodeList(children()[1]); };

    //! Returns the alias we set to the imported module
    StringRef name() const { return getPropertyStringDeref("name"); };

private:
    static void setContextForChildrenImpl(ImportName node);
    static NodeHandle semanticCheckImpl(ImportName node);
    static const char* toStringImpl(ImportName node);
};

/**
 * @brief      A module reference expression
 *
 * This is used to reference the content of another module. It is used as a base for all the import
 * names.
 *
 * Preconditions:
 *     - the given node must be the node containing the content of the referred module
 *
 * Postconditions:
 *     - one can access declarations from another module through this node
 */
struct ModuleRef : NodeHandle {
    DECLARE_NODE_COMMON(ModuleRef)

    /**
     * @brief      Creates a module reference node
     *
     * @param[in]  loc           The location of the node
     * @param[in]  moduleContent The content of module we are referring to
     *
     * @return     The desired node
     */
    static ModuleRef create(const Location& loc, NodeHandle moduleContent);

    //! Returns the module content we are referring to
    NodeHandle moduleContent() const { return referredNodes()[0]; };

private:
    static NodeHandle semanticCheckImpl(ModuleRef node);
};

} // namespace SprFrontend
