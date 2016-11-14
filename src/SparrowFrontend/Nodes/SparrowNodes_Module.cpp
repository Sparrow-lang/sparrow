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
        ASSERT(Nest_nodeArraySize(node->children) == 2);
        Node* moduleName = at(node->children, 0);
        // Node* imports = at(node->children, 1);
        Node* declarations = at(node->children, 1);

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

        // Add implicit import, if needed
        if ( g_implicitLibSC ) {
            // Add an using to the content of the implicit lib
            Location importLoc = node->location;
            Node* refImpContent = mkModuleRef(importLoc, g_implicitLibSC->mainNode);
            ASSERT(refImpContent);
            Node* starExp = mkStarExp(importLoc, refImpContent, fromCStr("*"));
            Node* iiCode = mkSprUsing(importLoc, StringRef({0,0}), starExp, privateAccess);

            Feather_addToNodeList(innerContent, iiCode);
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
}

void Module_SetContextForChildren(Node* node)
{
    // Expand the module node
    // We need to do it before setting the context, as packages would change the context
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

void ImportName_SetContextForChildren(Node* node)
{
    // We add the imported source code here to make sure that source codes are
    // loaded in the order they appear. We make sure that some LLVM code will
    // be loaded in the compile-time environment asap.

    Node* moduleName = at(node->children, 0);

    // Add the new source code to the compiler
    SourceCode* importedSc = addImportedSourceCode(node->location.sourceCode, moduleName);

    // Remember the main node of the imported SC
    Nest_setPropertyNode(node, "importedScMainNode", importedSc->mainNode);

    // Make sure this node is compiled whenever we search the symbol table of the current contenxt
    Nest_symTabAddToCheckNode(node->context->currentSymTab, node);

    Nest_defaultFunSetContextForChildren(node);
}

Node* ImportName_SemanticCheck(Node* node)
{
    Node* moduleName = at(node->children, 0);
    Node* declNames = at(node->children, 1);
    StringRef alias = Feather_hasName(node) ? Feather_getName(node) : StringRef({0, 0});

    AccessType accessType = getAccessType(node);
    if ( accessType == unspecifiedAccess )
        accessType = privateAccess;

    Node* importedScMainNode = Nest_getCheckPropertyNode(node, "importedScMainNode");

    Location importLoc = moduleName->location;

    // Reference to the content of the module we are importing
    if ( importedScMainNode->nodeKind != nkSparrowDeclModule ) {
        // Nothing to add for this kind of imported module
        return Feather_mkNop(node->location);
    }
    Node* refImpContent = mkModuleRef(importLoc, importedScMainNode);

    // By default imports are private

    Node* content = nullptr;

    // Get the usings that we need to add for this import
    if ( declNames ) {
        if ( declNames->nodeKind != nkFeatherNodeList )
            REP_INTERNAL(declNames->location, "Expected node list, found %1%") % Nest_nodeKindName(declNames);

        // For something like:
        //      import modName(f, g);
        // we return something like:
        //      {
        //          using f = <moduleRef>.f;
        //          using g = <moduleRef>.g;
        //      }
        for ( Node* id : all(declNames->children) ) {
            StringRef name = Feather_getName(id);
            Node* cid = mkCompoundExp(id->location, refImpContent, name);
            Node* usng = mkSprUsing(id->location, name, cid, accessType);
            content = Feather_addToNodeList(content, usng);
        }

        // Make sure we always have a content node
        if ( !content )
            content = Feather_mkNodeListVoid(importLoc, fromIniList({}));
    }
    else {
        // For something like:
        //      import modName;
        // we return something like:
        //      using <moduleRef>.*;
        Node* starExp = mkStarExp(importLoc, refImpContent, fromCStr("*"));
        content = mkSprUsing(importLoc, StringRef({0,0}), starExp, accessType);
    }

    // If we have an alias, create a package for it
    if ( size(alias) > 0 ) {
        content = mkSprPackage(importLoc, alias, content, accessType);
    }

    return content;
}

