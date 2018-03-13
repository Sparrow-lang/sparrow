#include <StdInc.h>
#include "SparrowNodes.h"
#include "SparrowNodesAccessors.h"

#include <SprDebug.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/QualifiedId.h>

#include "Nest/Api/SourceCode.h"
#include "Nest/Utils/NodeUtils.hpp"
#include "Nest/Utils/Alloc.h"

using namespace SprFrontend;
using namespace Nest;

SourceCode* g_implicitLibSC = nullptr;
SourceCode* g_compilerArgsSC = nullptr;

/// Given a module node, infer the the module name from the source code URL
StringRef inferModuleName(const char* url) {
    // Remove the path part
    const char* p = strrchr(url, '/');
    if (!p)
        p = strrchr(url, '\\');
    if (p)
        url = p + 1;
    // Remove the extension part
    p = strchr(url, '.');
    // Create a StringRef out of the selected part
    StringRef id = {url, p ? p : url + strlen(url)};
    return id;
}

namespace {
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
Node* createPackagesFromQid(const QidVec& qid, AccessType accessType, Node*& innerPackage) {

    innerPackage = nullptr;
    Node* pkg = nullptr;

    // Create the packages bottom-up
    for (int i = int(qid.size()) - 1; i >= 0; i--) {
        const auto& id = qid[i];

        Node* pkgContent =
                Feather_mkNodeListVoid(id.second, pkg ? fromIniList({pkg}) : fromIniList({}));
        pkg = mkSprPackage(id.second, id.first, pkgContent);
        setAccessType(pkg, accessType);

        if (!innerPackage)
            innerPackage = pkg;
    }

    return pkg;
}

/**
 * Create an implicit import the to given source code
 *
 * This will actually create just a using node pointing to the content of
 * the main node of the source code to import
 *
 * @param importLoc The location of the generated code
 * @param toImport  The source code to import
 *
 * @return Node that performs the import
 */
Node* createImplicitImport(Location importLoc, SourceCode* toImport) {
    // Add an using to the content of the imported source code
    Node* refImpContent = mkModuleRef(importLoc, toImport->mainNode);
    ASSERT(refImpContent);
    Node* starExp = mkStarExp(importLoc, refImpContent, fromCStr("*"));
    Node* iCode = mkSprUsing(importLoc, StringRef({nullptr, nullptr}), starExp);
    setAccessType(iCode, privateAccess);

    // Don't warn if we don't find anything
    Nest_setPropertyInt(iCode, propNoWarnIfNoDeclFound, 1);
    return iCode;
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
    if (moduleName) {
        modLoc = moduleName->location;
        interpretQualifiedId(moduleName, moduleNameQid);
    } else {
        modLoc = Nest_mkLocation1(node->location.sourceCode, 1, 1);
        ASSERT(node->location.sourceCode);
        ASSERT(node->location.sourceCode->url);
        StringRef name = inferModuleName(node->location.sourceCode->url);
        moduleNameQid.emplace_back(make_pair(name, modLoc));
    }
    if (moduleNameQid.empty() || size(moduleNameQid.back().first) == 0)
        REP_INTERNAL(modLoc, "Invalid module name");

    // Create the packages corresponding to the module name
    Node* innerMostPackage = nullptr;
    Node* outerContent = createPackagesFromQid(moduleNameQid, publicAccess, innerMostPackage);
    CHECK(modLoc, outerContent);
    CHECK(modLoc, innerMostPackage);

    // The inner content, without any packages -- a node list
    Node* innerContent = at(innerMostPackage->children, 0);

    // Add implicit import, if needed
    if (g_implicitLibSC) {
        Node* iCode = createImplicitImport(node->location, g_implicitLibSC);
        Feather_addToNodeList(innerContent, iCode);
    }

    // Add compiler defines import, if needed
    if (g_compilerArgsSC) {
        Node* iCode = createImplicitImport(node->location, g_compilerArgsSC);
        Feather_addToNodeList(innerContent, iCode);
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
    if (moduleName->nodeKind == nkSparrowExpLiteral) {
        if (!Literal_isString(moduleName))
            REP_INTERNAL(moduleName->location, "Invalid import name found %1%") %
                    Nest_toStringEx(moduleName);
        importedSc = Nest_addSourceCodeByFilename(curSourceCode, Literal_getData(moduleName));
        if (!importedSc) {
            REP_ERROR(moduleName->location, "Cannot import %1%") % Literal_getData(moduleName);
        }
    } else {
        QidVec qid;
        interpretQualifiedId(moduleName, qid);

        if (qid.empty())
            REP_ERROR(moduleName->location, "Nothing to import");

        // Transform qid into filename/dirname
        string filename;
        for (const auto& part : qid) {
            if (!filename.empty())
                filename += "/";
            filename += toString(part.first);
        }
        importedSc = Nest_addSourceCodeByFilename(curSourceCode, fromString(filename + ".spr"));
        if (!importedSc)
            REP_ERROR(moduleName->location, "Cannot import %1%") % (filename + ".spr");
    }
    return importedSc;
}
} // namespace

void Module_SetContextForChildren(Node* node) {
    // Expand the module node
    // We need to do it before setting the context, as packages would change the context
    Node* outerContent = expandModule(node);

    // Create a children context, and set it to the explanation of the module
    // This will create a symtab that it's independent of what we have so far
    CompilationContext* ctx = Nest_mkChildContextWithSymTab(node->context, node, modeUnspecified);
    Nest_setContext(outerContent, ctx);
}

Node* Module_SemanticCheck(Node* node) {
    node->type = Feather_getVoidType(modeCt);
    Node* content = Nest_getCheckPropertyNode(node, "spr.moduleOuterContent");
    return content;
}

void ImportName_SetContextForChildren(Node* node) {
    // We add the imported source code here to make sure that source codes are
    // loaded in the order they appear. We make sure that some LLVM code will
    // be loaded in the compile-time environment asap.

    Node* moduleName = at(node->children, 0);

    // If we loading the implicit library, keep the include order strict
    // We want includes to be processed asap, to be able to load the LLVM code
    // for the CT backend.
    if (!g_implicitLibSC) {
        // Add the new source code to the compiler
        SourceCode* importedSc = addImportedSourceCode(node->location.sourceCode, moduleName);
        if (!importedSc) {
            node->nodeError = 1;
            return;
        }

        // Remember the main node of the imported SC
        Nest_setPropertyNode(node, "importedScMainNode", importedSc->mainNode);
    }

    // Make sure this node is compiled whenever we search the symbol table of the current contenxt
    Nest_symTabAddToCheckNode(node->context->currentSymTab, node);

    Nest_defaultFunSetContextForChildren(node);
}

Node* ImportName_SemanticCheck(Node* node) {
    Node* moduleName = at(node->children, 0);
    Node* declNames = at(node->children, 1);
    StringRef alias = Feather_hasName(node) ? Feather_getName(node) : StringRef({nullptr, nullptr});

    AccessType accessType = getAccessType(node);
    if (accessType == unspecifiedAccess)
        accessType = privateAccess;

    // Get the main node of the imported source code
    Node* importedScMainNode = nullptr;
    if (Nest_hasProperty(node, "importedScMainNode")) {
        // Source code was already added; get its main node
        importedScMainNode = Nest_getCheckPropertyNode(node, "importedScMainNode");
    } else {
        // Add the new source code to the compiler
        SourceCode* importedSc = addImportedSourceCode(node->location.sourceCode, moduleName);
        if (!importedSc)
            return nullptr;
        importedScMainNode = importedSc->mainNode;
    }

    Location importLoc = moduleName->location;

    // Reference to the content of the module we are importing
    Node* refImpContent = mkModuleRef(importLoc, importedScMainNode);

    // By default imports are private

    Node* content = nullptr;

    // Get the usings that we need to add for this import
    if (declNames) {
        if (declNames->nodeKind != nkFeatherNodeList)
            REP_INTERNAL(declNames->location, "Expected node list, found %1%") %
                    Nest_nodeKindName(declNames);

        // For something like:
        //      import modName(f, g);
        // we return something like:
        //      {
        //          using f = <moduleRef>.f;
        //          using g = <moduleRef>.g;
        //      }
        for (Node* id : all(declNames->children)) {
            StringRef name = Feather_getName(id);
            Node* cid = mkCompoundExp(id->location, refImpContent, name);
            Node* usng = mkSprUsing(id->location, name, cid);
            setAccessType(usng, accessType);
            content = Feather_addToNodeList(content, usng);
        }

        // Make sure we always have a content node
        if (!content)
            content = Feather_mkNodeListVoid(importLoc, fromIniList({}));
    } else {
        // For something like:
        //      import modName;
        // we return something like:
        //      using <moduleRef>.*;
        Node* starExp = mkStarExp(importLoc, refImpContent, fromCStr("*"));
        content = mkSprUsing(importLoc, StringRef({nullptr, nullptr}), starExp);
        setAccessType(content, accessType);

        // Don't warn if we don't find anything
        Nest_setPropertyInt(content, propNoWarnIfNoDeclFound, 1);
    }

    // If we have an alias, create a package for it
    if (size(alias) > 0) {
        content = mkSprPackage(importLoc, alias, content);
        setAccessType(content, accessType);
    }

    return content;
}

const char* ImportName_toString(const Node* node) {
    Node* moduleName = at(node->children, 0);
    Node* declNames = at(node->children, 1);
    StringRef alias = Feather_hasName(node) ? Feather_getName(node) : StringRef({nullptr, nullptr});

    ostringstream os;
    os << "ImportName(" << moduleName << ", " << declNames;
    if (alias.begin && size(alias) > 0)
        os << ", \"" << alias.begin << "\"";
    os << ")";
    return dupString(os.str().c_str());
}
