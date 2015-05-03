#pragma once

#include "FrontendFactory.h"

namespace Nest
{
    /// Simple implementation for the frontend factory
    class FrontendFactoryImpl : public FrontendFactory
    {
    public:
        virtual SourceCode* createParser(const string& filename);
        virtual SourceCode* createParser(const string& language, const string& filename);

        virtual void registerParser(const string& language, const vector<string>& extensions, const CreateParserFn& createFn);
        virtual void registerParser(const string& language, const string& extension, const CreateParserFn& createFn);

    private:
        /// Map from extensions string to (language, create-fn) pair
        unordered_multimap<string, pair<string, CreateParserFn> > extensionsMap_;
    };
}
