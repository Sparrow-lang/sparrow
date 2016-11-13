#include <StdInc.h>
#include "SparrowNodes.h"
#include "SparrowNodesAccessors.h"

#include <SprDebug.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/QualifiedId.h>

#include "Nest/Api/SourceCode.h"
#include "Nest/Utils/NodeUtils.hpp"

using namespace SprFrontend;
using namespace Nest;

SourceCode* g_implicitLibSC = nullptr;

/// Given a module node, infer the the module name from the source code URL
StringRef inferModuleName(Node* node) {
    ASSERT(node->location.sourceCode);
    ASSERT(node->location.sourceCode->url);
    const char* url = node->location.sourceCode->url;
    // Remove the path part
    const char* p = strrchr(url, '/');
    if ( !p )
        p = strrchr(url, '\\');
    if ( p )
        url = p+1;
    // Remove the extension part
    p = strchr(url, '.');
    // Create a StringRef out of the selected part
    StringRef id = { url, p ? p : url + strlen(url) };
    return id;
}

namespace
{
    /**
     * Create a structure of nested packages corresponding to the given QID
     *
     * Does not check to reuse any of the existing package. The assumption is
     * that there is nothing created yet.
     *
     * We use this to create the packages for the module.
     *
     * qid needs to have at least one element.
     *
     * @param qid                   The list of names for the packages
     * @param accessType            The access type to be used for creating the packages
     * @param [out] innerPackage    The inner most package created
     *
     * @return The inner most package created/reused
     */
    Node* createPackagesFromQid(const QidVec& qid,
                                AccessType accessType, Node*& innerPackage) {

        innerPackage = nullptr;
        Node* pkg = nullptr;

        // Create the packages bottom-up
        for ( int i = int(qid.size())-1; i>=0; i-- ) {
            const auto& id = qid[i];

            Node* pkgContent = Feather_mkNodeListVoid(id.second, pkg ? fromIniList({ pkg }) : fromIniList({}));
            pkg = mkSprPackage(id.second, id.first, pkgContent, accessType);

            if ( !innerPackage )
                innerPackage = pkg;
        }

        return pkg;
    }

    /**
     * Structure describing info about a package into a hierarchy of packages.
     *
     * Instead of holding pointers to the actual package nodes, we will hold
     * pointers to the content of the packages (their internal node-list).
     *
     * The top-level package will contain a null parent and an empty name.
     */
    struct PackageDesc {
        Node* parent;   //!< The parent package content node
        StringRef name; //!< The name of the package
        Node* content;  //!< The node that holds the content of the package (node-list)
    };

    /// Descriptions to the packages we've encountered
    /// The first entry will represent the top-level package
    typedef vector<PackageDesc> Packages;

    /**
     * Find or create a package node
     *
     * This makes sure that we don't create the same package multiple times. It
     * records each package that we've created, so that we can reuse them.
     *
     * If a package is not found, we create it here.
     * This will also create the content of the package, a node-list.
     *
     * We have a relatively small number of packages, so linear operations sound
     * good for our purpose.
     *
     * @param packages      The list of existing packages
     * @param parent        The package content that contains the requested package
     * @param name          The name of the package to be found/created
     * @param loc           The location to be used for this package
     * @param accessType    The access type for the package
     *
     * @return The content node of the required package
     */
    Node* findOrCreatePackage(Packages& packages, Node* parent, StringRef name, Location loc, AccessType accessType) {
        // Try to find the package to create
        for ( auto& pk : packages ) {
            if ( pk.parent == parent && pk.name == name )
                return pk.content; // Package already exiting
        }

        // Create a new package (and add it to the parent)
        Node* pkgContent = Feather_mkNodeListVoid(loc, fromIniList({}));
        Node* pkg = mkSprPackage(loc, name, pkgContent, accessType);
        ASSERT(parent);
        Feather_addToNodeList(parent, pkg);

        // Add a new entry to our list of packages descriptions
        packages.emplace_back(PackageDesc{parent, name, pkgContent});

        return pkgContent;
    }

    /**
     * Create/reuse a structure of nested packages corresponding to the given QID
     *
     * If qid is empty, then this will return the node-list in which all the packages are placed
     *
     * @param qid           The list of names for the packages
     * @param loc           The location to be used for creating the packages
     * @param accessType    The access type to be used for creating the packages
     * @param packages      The list of existing packages to be used/updated
     *
     * @return The content node (node-list) of the inner most package created/reused
     */
    Node* createReusePackages(const QidVec& qid, Location loc,
            AccessType accessType, Packages& packages) {

        // Start searching from the first package
        Node* pkgContent = packages[0].content;

        // Create or reuse package top-down
        for ( const auto& id : qid ) {
            pkgContent = findOrCreatePackage(packages, pkgContent, id.first, loc, accessType);
        }

        return pkgContent;
    }

    /// Given an import module name, add the corresponding source code to the compiler
    /// Returns the created SourceCode object
    SourceCode* addImportedSourceCode(const SourceCode* curSourceCode, Node* moduleName) {
        ASSERT(moduleName);
        SourceCode* importedSc;
        if ( moduleName->nodeKind == nkSparrowExpLiteral ) {
            if ( !Literal_isString(moduleName) )
                REP_INTERNAL(moduleName->location, "Invalid import name found %1%") % Nest_toStringEx(moduleName);
            importedSc = Nest_addSourceCodeByFilename(curSourceCode, Literal_getData(moduleName));
        }
        else
        {
            QidVec qid;
            interpretQualifiedId(moduleName, qid);

            if ( qid.empty() )
                REP_INTERNAL(moduleName->location, "Nothing to import");

            // Transform qid into filename/dirname
            string filename;
            for ( const auto& part: qid )
            {
                if ( !filename.empty() )
                    filename += "/";
                filename += toString(part.first);
            }
            importedSc = Nest_addSourceCodeByFilename(curSourceCode, fromString(filename + ".spr"));
        }
        if ( !importedSc )
            REP_INTERNAL(moduleName->location, "Cannot import %1%") % moduleName;
        return importedSc;
    }

    /**
     * Adds the code supporting an import to the current source code.
     *
     * @param curSourceCode The source code of the current module
     * @param packages      [in/out] The list of packages to be created for the current module
     * @param importLoc     The location from which this import is added (into the current source code)
     * @param importedSc    The source code to be imported
     * @param declNames     Node list of identifiers describing the decls to be imported
     * @param equals        True if this is an equals import (e.g., renamed import)
     * @param alias         The alias passed to the import (maybe empty)
     * @param accessType    The access type to be used for the import
     */
    void addImportCode(const SourceCode* curSourceCode, Packages& packages,
            Location importLoc, const SourceCode* importedSc,
            Node* declNames, bool equals, StringRef alias,
            AccessType accessType = unspecifiedAccess) {

        // By default imports are private
        if ( accessType == unspecifiedAccess )
            accessType = privateAccess;

        // Reference to the content of the module we are importing
        Node* refImpContent = nullptr;
        if ( importedSc->mainNode->nodeKind != nkSparrowDeclModule ) {
            if ( size(alias) > 0 )
                REP_ERROR(importLoc, "Cannot use import with alias for non Sparrow files");
            if ( equals )
                REP_ERROR(importLoc, "Cannot use equal import for non Sparrow files");
            if ( declNames && size(declNames->children) > 0 )
                REP_ERROR(importLoc, "Cannot use selective import for non Sparrow files");
            return;
        }
        else {
            refImpContent = mkModuleRef(importLoc, importedSc->mainNode);
        }

        // Get the QID used to refer the module
        equals = true;     // TODO(modules): Change this
        QidVec refQid;
        if ( equals ) {
            // The given alias is the QID used to access the imported module
            if ( size(alias) > 0 )
                refQid.emplace_back(make_pair(alias, importLoc));
            // If no 'alias' is given, refQid will be empty, and no extra package will be created for the import
        }
        else {
            // The QID is the name of the imported module
            Node* impModName = nullptr;
            Node* impMainNode = importedSc->mainNode;
            if ( impMainNode && impMainNode->nodeKind == nkSparrowDeclModule ) {
                impModName = at(impMainNode->children, 0);
            }
            if ( impModName ) {
                interpretQualifiedId(impModName, refQid);
            }
            else {
                StringRef inferredName = inferModuleName(impMainNode);
                refQid.emplace_back(make_pair(inferredName, importLoc));
            }
            if ( refQid.empty() ) {
                Location loc = impModName ? impModName->location : Nest_mkLocation1(importedSc, 1, 1);
                REP_INTERNAL(loc, "Invalid qualified ID for module name (%1%)") % Nest_toStringEx(impModName);
            }
        }

        // Drop the end name in the classic (non-equals) case: 'using foo.bar'
        pair<StringRef, Location> lastId;
        if ( !declNames && !equals ) {
            ASSERT(!refQid.empty());
            lastId = refQid.back();
            refQid.pop_back();
        }

        // Create the required packages
        Node* pkgContent = createReusePackages(refQid, importLoc, accessType, packages);

        // The using(s) that we need to add
        if ( declNames ) {
            if ( declNames->nodeKind != nkFeatherNodeList )
                REP_INTERNAL(declNames->location, "Expected node list, found %1%") % Nest_nodeKindName(declNames);

            // For something like:
            //      import foo.bar(f, g);
            // we create something like:
            //      package foo { package bar {
            //          using f = <foobarContentRef>.f;
            //          using g = <foobarContentRef>.g;
            //      }}
            for ( Node* id : all(declNames->children) ) {
                StringRef name = Feather_getName(id);
                Node* cid = mkCompoundExp(id->location, refImpContent, name);
                Feather_addToNodeList(pkgContent, mkSprUsing(id->location, name, cid, accessType));

            }
        } else if ( !equals ) {
            // For something like:
            //      import foo.bar;
            // we create something like:
            //      package foo {
            //          using bar = <foobarContentRef>;
            //      }

            // Create a using towards the imported module name, using the last
            // name from the QID
            Feather_addToNodeList(pkgContent, mkSprUsing(lastId.second, lastId.first, refImpContent, accessType));
        } else {
            // For something like:
            //      import =foo.bar;
            // we create something like:
            //      using<foobarContentRef>.*;

            Node* starExp = mkStarExp(importLoc, refImpContent, fromCStr("*"));
            Feather_addToNodeList(pkgContent, mkSprUsing(importLoc, StringRef({0,0}), starExp, accessType));
        }
    }

    /**
     * Handle an import; add the new source code; add the code corresponding to current source code
     *
     * @param curSourceCode The source code of the current module
     * @param packages      [in/out] The list of packages to be created for the current module
     * @param importName    The import name describing what to import
     */
    void handleImport(const SourceCode* curSourceCode, Packages& packages, Node* importName) {
        if ( importName->nodeKind != nkSparrowDeclImportName ) {
            Nest_reportFmt(importName->location, diagInternalError, "Expected spr.importName node; found %s", Nest_nodeKindName(importName));
            return;
        }

        Node* moduleName = at(importName->children, 0);
        Node* declNames = at(importName->children, 1);
        bool equals = 0 != Feather_hasName(importName);
        StringRef alias = equals ? Feather_getName(importName) : StringRef({0, 0});
        AccessType accessType = getAccessType(importName);

        // Add the new source code to the compiler
        SourceCode* importedSc = addImportedSourceCode(curSourceCode, moduleName);

        // Now actually handle the import in the current source code
        addImportCode(curSourceCode, packages, moduleName->location, importedSc, declNames, equals, alias, accessType);
    }

    void handleImplicitImport(const SourceCode* curSourceCode, Packages& packages) {
        if ( g_implicitLibSC ) {
            Location impModuleLoc = Nest_mkLocation1(curSourceCode, 1, 1);
            addImportCode(curSourceCode, packages, impModuleLoc, g_implicitLibSC, nullptr, true, StringRef{0, 0});
        }
    }

    /**
     * Given a module node, this will generate the appropriate code for it
     *
     * It will first create the top-level packages corresponding to the module
     * name. Then it will check the imports of the module and expand them into
     * the appropriate content
     *
     * @param node The node describing the module
     *
     * @return The node that the module will expand to
     */
    Node* expandModule(Node* node) {
        ASSERT(Nest_nodeArraySize(node->children) == 3);
        Node* moduleName = at(node->children, 0);
        Node* imports = at(node->children, 1);
        Node* declarations = at(node->children, 2);

        Location modLoc;

        // Get the QID representing the module
        // If we don't have a module name, infer it from the name of the file
        QidVec moduleNameQid;
        if ( moduleName ) {
            modLoc = moduleName->location;
            interpretQualifiedId(moduleName, moduleNameQid);
        } else {
            modLoc = Nest_mkLocation1(node->location.sourceCode, 1, 1);
            StringRef name = inferModuleName(node);
            moduleNameQid.emplace_back(make_pair(name, modLoc));
        }
        if ( moduleNameQid.empty() || size(moduleNameQid.back().first) == 0 )
            REP_INTERNAL(modLoc, "Invalid module name");

        // Create the packages corresponding to the module name
        Node* innerMostPackage = nullptr;
        Node* outerContent = createPackagesFromQid(moduleNameQid, publicAccess, innerMostPackage);
        CHECK(modLoc, outerContent);
        CHECK(modLoc, innerMostPackage);

        // The inner content, without any packages -- a node list
        Node* innerContent = at(innerMostPackage->children, 0);

        // Handle imports
        if ( imports || g_implicitLibSC ) {
            // The packages that we want to import
            // Add one entry to indicate the top-level without any package
            Packages packages;
            packages.emplace_back(PackageDesc{nullptr, fromCStr(""), innerContent});

            // Add the implicit import
            handleImplicitImport(node->location.sourceCode, packages);

            // Accumulate everything that we need to add
            if ( imports ) {
                for ( Node* i: imports->children )
                    handleImport(node->location.sourceCode, packages, i);
            }
        }

        // Add the declarations to the inner content
        // We do this after adding imports code
        Feather_addToNodeList(innerContent, declarations);

        // The resulting decl is the inner most package created for this module
        Nest_setPropertyNode(node, propResultingDecl, innerMostPackage);

        // We already know the explanation of this node
        Nest_setPropertyNode(node, "spr.moduleOuterContent", outerContent);

        return outerContent;
    }
}

void Module_SetContextForChildren(Node* node)
{
    // Expand the module node
    Node* outerContent = expandModule(node);

    // Create a children context, and set it to the explanation of the module
    // This will create a symtab that it's independent of what we have so far
    CompilationContext* ctx = Nest_mkChildContextWithSymTab(node->context, node, modeUnspecified);
    Nest_setContext(outerContent, ctx);
}

Node* Module_SemanticCheck(Node* node)
{
    node->type = Feather_getVoidType(modeCt);
    Node* content =  Nest_getCheckPropertyNode(node, "spr.moduleOuterContent");

    return content;
}

