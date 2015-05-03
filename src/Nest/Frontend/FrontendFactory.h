#pragma once

#include <functional>

FWD_CLASS1(Nest, SourceCode);

namespace Nest
{
    /// Interface for a factory class that creates parsers for different languages, based on the file extension or language
    class FrontendFactory
    {
    public:
        /// Functor types that creates backend objects
        typedef function<SourceCode* (const string& filename)> CreateParserFn;

        virtual ~FrontendFactory() {}

        /// Create a parser for the given filename, based on the extension of the filename
        virtual SourceCode* createParser(const string& filename) = 0;

        /// Create a parser for the given filename, of the specified language (ignoring the extension of the file)
        virtual SourceCode* createParser(const string& language, const string& filename) = 0;

        /// Registers a parser, with language and extensions
        /// The languages must be registered first, before being able to create parsers for the given languages
        virtual void registerParser(const string& language, const vector<string>& extensions, const CreateParserFn& createFn) = 0;

        /// Registers a parser, with language and one extension
        /// The languages must be registered first, before being able to create parsers for the given languages
        virtual void registerParser(const string& language, const string& extension, const CreateParserFn& createFn) = 0;
    };
}
