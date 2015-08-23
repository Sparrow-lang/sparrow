#pragma once

#include "Compiler.h"
#include "CompilerSettings.h"

#include <Nest/Intermediate/NodeVector.h>

#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <vector>
#include <string>

namespace Nest
{
    /// The main class of the compiler platform, which represents the actual compiler.
    /// Holds the main compiler objects
    class CompilerImpl : public Compiler
    {
    public:
        CompilerImpl();
        ~CompilerImpl();

        virtual CompilerSettings& settings();
        virtual const CompilerSettings& settings() const;

        virtual Common::DiagnosticReporter& diagnosticReporter() const;
        virtual CompilationContext* rootContext() const;
        virtual Backend& backend() const;

    public:
        virtual void createBackend(const char* mainFilename);
        virtual void compileFile(const string& filename);
        virtual void addSourceCodeByFilename(const SourceCode* orig, string filename);
        virtual void addSourceCodeByQid(const SourceCode* orig, vector<string> qid);
        virtual const SourceCode* getSourceCodeForFilename(const string& filename);
        virtual void queueSemanticCheck(Node* node);
        virtual void ctProcess(Node* node);
        virtual Node* ctEval(Node* node);
        virtual size_t sizeOf(TypeRef type);
        virtual size_t alignmentOf(TypeRef type);

    private:
        struct ImportInfo
        {
            const SourceCode* originSourceCode_;
            boost::filesystem::path filename_;
            vector<string> qid_;

            ImportInfo(const SourceCode* orig, string filename)
                : originSourceCode_(orig), filename_(move(filename))
            {}

            ImportInfo(const SourceCode* orig, string filename, vector<string> qid)
                : originSourceCode_(orig), filename_(), qid_(move(qid))
            {}
        };

        void semanticCheckNodes();
        void handleImport(const ImportInfo& import);
        bool checkFileDir(const ImportInfo& import);
        void handleDirectory(const ImportInfo& import);
        bool handleImportFile(const ImportInfo& import);

    private:
        CompilerSettings settings_;
        Common::DiagnosticReporter* diagnosticReporter_;
        CompilationContext* rootContext_;
        Backend* backend_;

        /// The path of the current directory
        boost::filesystem::path curPath_;

        /// List of source codes parsed by this class
        unordered_map<const SourceCode*, boost::filesystem::path> sourceCodes_;

        /// The files that were handled before
        unordered_set<string> handledFiles_;

        /// List of source codes to compile
        vector<SourceCode*> toCompile_;

        /// The imports that were not handled, as no appropriate source code was found
        /// We associate these imports to the originating source code
        unordered_map<const SourceCode*, vector<ImportInfo> > unhandledImports_;

        /// List of nodes to be semantically checked
        NodeVector toSemanticCheck_;
    };
}
