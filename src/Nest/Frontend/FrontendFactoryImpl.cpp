#include <StdInc.h>
#include "FrontendFactoryImpl.h"

#include <boost/filesystem.hpp>

using namespace Nest;

namespace
{
    string extensionFromFilename(const string& filename)
    {
        boost::filesystem::path p(filename);
        return p.extension().string();
    }
}

SourceCode* FrontendFactoryImpl::createParser(const string& filename)
{
    const string& ext = extensionFromFilename(filename);
    auto itPair = extensionsMap_.equal_range(ext);
    if ( itPair.first == itPair.second )
        return nullptr;
    else
    {
        const CreateParserFn& fn = itPair.first->second.second;
        return fn(filename);
    }
}

SourceCode* FrontendFactoryImpl::createParser(const string& language, const string& filename)
{
    const string& ext = extensionFromFilename(filename);
    auto itPair = extensionsMap_.equal_range(ext);
    if ( itPair.first == itPair.second )
        return nullptr;
    else
    {
        auto it = itPair.first;
        auto ite = itPair.second;
        for ( ; it!=ite; ++it )
        {
            if ( it->second.first == language )
            {
                const CreateParserFn& fn = it->second.second;
                return fn(filename);
            }
        }
        return nullptr;
    }
}

void FrontendFactoryImpl::registerParser(const string& language, const vector<string>& extensions, const CreateParserFn& createFn)
{
    for ( const string& ext: extensions )
    {
        extensionsMap_.insert(make_pair(ext, make_pair(language, createFn)));
    }
}

void FrontendFactoryImpl::registerParser(const string& language, const string& extension, const CreateParserFn& createFn)
{
    extensionsMap_.insert(make_pair(extension, make_pair(language, createFn)));
}
