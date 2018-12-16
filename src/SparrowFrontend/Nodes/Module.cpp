#include "SparrowFrontend/StdInc.h"
#include "SparrowFrontend/Nodes/Module.hpp"
#include "SparrowFrontend/Nodes/Decl.hpp"
#include "SparrowFrontend/Nodes/Exp.hpp"
#include "SparrowFrontend/Nodes/AccessType.h"
#include "SparrowFrontend/SprDebug.h"
#include "SparrowFrontend/Helpers/QualifiedId.h"
#include "SparrowFrontend/Helpers/DeclsHelpers.h"

#include "Feather/Utils/cppif/FeatherNodes.hpp"

#include "Nest/Api/SourceCode.h"
#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/cppif/NodeHelpers.hpp"

Nest_SourceCode* g_implicitLibSC = nullptr;
Nest_SourceCode* g_compilerArgsSC = nullptr;

//! Given a module node, infer the the module name from the source code URL
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

namespace SprFrontend {

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
 * @param qid          The list of names for the packages
 * @param innerContent The content of the inner-most package
 *
 * @return Pair of outer-most, inner-most package created
 */
pair<PackageDecl, PackageDecl> createPackagesFromQid(const QidVec& qid, NodeList innerContent) {

    PackageDecl inner;
    PackageDecl outer;

    // Create the packages bottom-up
    for (int i = int(qid.size()) - 1; i >= 0; i--) {
        const auto& id = qid[i];

        auto body = outer ? Feather::NodeList::create(id.second, NodeRange({outer}), true)
                          : innerContent;
        outer = PackageDecl::create(id.second, id.first, body);
        setAccessType(outer, publicAccess);

        if (!inner)
            inner = outer;
    }

    return make_pair(outer, inner);
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
NodeHandle createImplicitImport(const Location& importLoc, Nest_SourceCode* toImport) {
    // Add an using to the content of the imported source code
    auto refImpContent = ModuleRef::create(importLoc, toImport->mainNode);
    ASSERT(refImpContent);
    NodeHandle starExp = StarExp::create(importLoc, refImpContent, StringRef("*"));
    auto iCode = UsingDecl::create(importLoc, StringRef{}, starExp);
    setAccessType(iCode, privateAccess);

    // Don't warn if we don't find anything
    iCode.setProperty(propNoWarnIfNoDeclFound, 1);
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
NodeHandle expandModule(Module node) {
    NodeHandle moduleName = node.moduleName();
    NodeList declarations = node.decls();

    Location modLoc{};

    // Get the QID representing the module
    // If we don't have a module name, infer it from the name of the file
    QidVec moduleNameQid;
    if (moduleName) {
        modLoc = moduleName.location();
        interpretQualifiedId(moduleName, moduleNameQid);
    } else {
        modLoc = Nest_mkLocation1(node.location().sourceCode, 1, 1);
        ASSERT(node.location().sourceCode);
        ASSERT(node.location().sourceCode->url);
        StringRef name = inferModuleName(node.location().sourceCode->url);
        moduleNameQid.emplace_back(make_pair(name, modLoc));
    }
    if (moduleNameQid.empty() || StringRef(moduleNameQid.back().first).empty())
        REP_INTERNAL(modLoc, "Invalid module name");

    // Create the inner content node; here we add the declarations of the module
    NodeList innerContent = NodeList::create(modLoc, NodeRange{}, true);

    // Create the packages corresponding to the module name
    PackageDecl innerMostPackage, outerContent;
    tie(outerContent, innerMostPackage) = createPackagesFromQid(moduleNameQid, innerContent);
    CHECK(modLoc, outerContent);
    CHECK(modLoc, innerMostPackage);

    // Add implicit import, if needed
    if (g_implicitLibSC) {
        auto iCode = createImplicitImport(node.location(), g_implicitLibSC);
        innerContent.addChild(iCode);
    }

    // Add compiler defines import, if needed
    if (g_compilerArgsSC) {
        auto iCode = createImplicitImport(node.location(), g_compilerArgsSC);
        innerContent.addChild(iCode);
    }

    // Add the declarations to the inner content
    // We do this after adding imports code
    innerContent.addChild(declarations);

    // The resulting decl is the inner most package created for this module
    node.setProperty(propResultingDecl, innerMostPackage);

    // We already know the explanation of this node
    node.setProperty("spr.moduleOuterContent", outerContent);

    return outerContent;
}

//! Add the given filename to the compiler, and return its sourcecode
//! Check for errors
Nest_SourceCode* checkAddSourceCode(
        const Location& loc, const Nest_SourceCode* curSourceCode, StringRef filename) {
    auto importedSc = Nest_addSourceCodeByFilename(curSourceCode, filename);
    if (!importedSc)
        REP_ERROR(loc, "Cannot import %1%") % filename;
    return importedSc;
}

//! Given an import module name, add the corresponding source code to the compiler
//! Returns the created SourceCode object
Nest_SourceCode* addImportedSourceCode(
        const Nest_SourceCode* curSourceCode, NodeHandle moduleName) {
    Literal modNameLit = moduleName.kindCast<Literal>();
    if (modNameLit) {
        if (!modNameLit.isString())
            REP_INTERNAL(moduleName.location(), "Invalid import name found %1%") %
                    Nest_toStringEx(moduleName);
        return checkAddSourceCode(moduleName.location(), curSourceCode, modNameLit.dataStr());
    } else {
        QidVec qid;
        interpretQualifiedId(moduleName, qid);

        if (qid.empty())
            REP_ERROR(moduleName.location(), "Nothing to import");

        // Transform qid into filename/dirname
        string filename;
        for (const auto& part : qid) {
            if (!filename.empty())
                filename += "/";
            filename += part.first.begin;
        }
        return checkAddSourceCode(
                moduleName.location(), curSourceCode, StringRef(filename + ".spr"));
    }
}
} // namespace

DEFINE_NODE_COMMON_IMPL(Module, DeclNode)

Module Module::create(const Location& loc, NodeHandle moduleName, NodeList decls) {
    return Nest::createNode<Module>(loc, NodeRange({moduleName, decls}));
}

void Module::setContextForChildrenImpl(Module node) {
    // Expand the module node
    // We need to do it before setting the context, as packages would change the context
    NodeHandle outerContent = expandModule(node);

    // Create a children context, and set it to the explanation of the module
    // This will create a symtab that it's independent of what we have so far
    CompilationContext* ctx = Nest_mkChildContextWithSymTab(node.context(), node, modeRt);
    outerContent.setContext(ctx);
}

NodeHandle Module::semanticCheckImpl(Module node) {
    node.setType(Feather::VoidType::get(modeRt));
    return node.getCheckPropertyNode("spr.moduleOuterContent");
}

DEFINE_NODE_COMMON_IMPL(ImportName, DeclNode)

ImportName ImportName::create(
        const Location& loc, NodeHandle moduleName, NodeList importedDeclNames, StringRef alias) {
    REQUIRE_NODE(loc, moduleName);
    auto res = Nest::createNode<ImportName>(loc, NodeRange({moduleName, importedDeclNames}));
    if (alias)
        res.setProperty("name", alias);
    return res;
}

void ImportName::setContextForChildrenImpl(ImportName node) {
    // We add the imported source code here to make sure that source codes are
    // loaded in the order they appear. We make sure that some LLVM code will
    // be loaded in the compile-time environment asap.

    NodeHandle moduleName = node.moduleName();

    // If we loading the implicit library, keep the include order strict
    // We want includes to be processed asap, to be able to load the LLVM code
    // for the CT backend.
    if (!g_implicitLibSC) {
        // Add the new source code to the compiler
        Nest_SourceCode* importedSc = addImportedSourceCode(node.location().sourceCode, moduleName);
        if (!importedSc) {
            node.handle->nodeError = 1;
            return;
        }

        // Remember the main node of the imported SC
        node.setProperty("importedScMainNode", NodeHandle(importedSc->mainNode));
    }

    // Make sure this node is compiled whenever we search the symbol table of the current context
    Nest_symTabAddToCheckNode(node.context()->currentSymTab, node);

    DeclNode::setContextForChildrenImpl(node);
}

NodeHandle ImportName::semanticCheckImpl(ImportName node) {
    NodeHandle moduleName = node.moduleName();
    NodeList declNames = node.importedDeclNames();
    StringRef alias = node.name();

    AccessType accessType = getAccessType(node);
    if (accessType == unspecifiedAccess)
        accessType = privateAccess;

    // Get the main node of the imported source code
    NodeHandle importedScMainNode;
    if (node.hasProperty("importedScMainNode")) {
        // Source code was already added; get its main node
        importedScMainNode = node.getCheckPropertyNode("importedScMainNode");
    } else {
        // Add the new source code to the compiler
        Nest_SourceCode* importedSc = addImportedSourceCode(node.location().sourceCode, moduleName);
        if (!importedSc)
            return {};
        importedScMainNode = importedSc->mainNode;
    }

    Location importLoc = moduleName.location();

    // Reference to the content of the module we are importing
    ModuleRef refImpContent = ModuleRef::create(importLoc, importedScMainNode);

    NodeHandle content;

    // Get the usings that we need to add for this import
    if (declNames) {
        // For something like:
        //      import modName(f, g);
        // we return something like:
        //      {
        //          using f = <moduleRef>.f;
        //          using g = <moduleRef>.g;
        //      }
        for (NodeHandle id : declNames.children()) {
            StringRef name = Feather_getName(id);
            auto cid = CompoundExp::create(id.location(), refImpContent, name);
            auto usng = UsingDecl::create(id.location(), name, cid);
            setAccessType(usng, accessType);
            content = NodeList::append(NodeList(content), usng);
        }

        // Make sure we always have a content node
        if (!content)
            content = Feather::NodeList::create(importLoc, NodeRange{}, true);
    } else {
        // For something like:
        //      import modName;
        // we return something like:
        //      using <moduleRef>.*;

        auto starExp = StarExp::create(importLoc, refImpContent, StringRef("*"));
        auto usng = UsingDecl::create(importLoc, StringRef{}, starExp);
        setAccessType(usng, accessType);

        // Don't warn if we don't find anything
        usng.setProperty(propNoWarnIfNoDeclFound, 1);

        content = usng;
    }

    // If we have an alias, create a package for it
    if (alias) {
        content = PackageDecl::create(importLoc, alias, content);
        setAccessType(content, accessType);
    }

    return content;
}
const char* ImportName::toStringImpl(ImportName node) {
    auto moduleName = node.moduleName();
    auto declNames = node.importedDeclNames();
    StringRef alias = node.name();

    ostringstream os;
    os << "ImportName(" << moduleName << ", " << declNames;
    if (alias)
        os << ", \"" << alias.begin << "\"";
    os << ")";
    return dupString(os.str().c_str());
}

DEFINE_NODE_COMMON_IMPL(ModuleRef, NodeHandle)

ModuleRef ModuleRef::create(const Location& loc, NodeHandle module) {
    REQUIRE_NODE(loc, module);
    return Nest::createNode<ModuleRef>(loc, NodeRange{}, NodeRange({module}));
}

NodeHandle ModuleRef::semanticCheckImpl(ModuleRef node) {
    auto module = node.moduleContent();
    if (!module)
        return {};

    if (module.kind() == nkSparrowDeclModule) {
        // If we are referring a Sparrow module, point to the inner most package
        auto innerMostPackage = module.getCheckPropertyNode(propResultingDecl);
        node.setProperty(propResultingDecl, innerMostPackage);
    } else {
        // Point to the main node; assume it's a package or something
        node.setProperty(propResultingDecl, module);
    }

    node.setType(Feather::VoidType::get(node.context()->evalMode));
    return node; // This node should never be translated directly
}

} // namespace SprFrontend
