#include <StdInc.hpp>
#include "SourceCodeKindRegistrar.h"

#include <boost/filesystem.hpp>

namespace
{
    string extensionFromFilename(const string& filename)
    {
        boost::filesystem::path p(filename);
        return p.extension().string();
    }
}

namespace
{
    struct SourceCodeKindDescription
    {
        const char* extension;
        const char* description;
        const char* extraInfo;
        FParseSourceCode funParseSourceCode;
        FGetSourceCodeLine funGetSourceCodeLine;
        FTranslateCtToRt funTranslateCtToRt;
    };

    /// The registered SourceCode kinds
    static vector<SourceCodeKindDescription> registeredSourceCodeKinds;
}

int Nest_registerSourceCodeKind(
        const char* extension,
        const char* description,
        const char* extraInfo,
        FParseSourceCode funParseSourceCode,
        FGetSourceCodeLine funGetSourceCodeLine,
        FTranslateCtToRt funTranslateCtToRt)
{
    int kind = registeredSourceCodeKinds.size();
    SourceCodeKindDescription desc = { extension, description, extraInfo, funParseSourceCode, funGetSourceCodeLine, funTranslateCtToRt };
    registeredSourceCodeKinds.push_back(desc);
    return kind;
}

const char* Nest_getSourceCodeExtension(int sourceCodeKind)
{
    ASSERT(0 <= sourceCodeKind  && sourceCodeKind < registeredSourceCodeKinds.size());
    return registeredSourceCodeKinds[sourceCodeKind].extension;
}

const char* Nest_getSourceCodeDescription(int sourceCodeKind)
{
    ASSERT(0 <= sourceCodeKind  && sourceCodeKind < registeredSourceCodeKinds.size());
    return registeredSourceCodeKinds[sourceCodeKind].description;
}

const char* Nest_getSourceCodeExtraInfo(int sourceCodeKind)
{
    ASSERT(0 <= sourceCodeKind  && sourceCodeKind < registeredSourceCodeKinds.size());
    return registeredSourceCodeKinds[sourceCodeKind].extraInfo;
}

FParseSourceCode Nest_getParseSourceCodeFun(int sourceCodeKind)
{
    ASSERT(0 <= sourceCodeKind  && sourceCodeKind < registeredSourceCodeKinds.size());
    return registeredSourceCodeKinds[sourceCodeKind].funParseSourceCode;
}

FGetSourceCodeLine Nest_getGetSourceCodeLineFun(int sourceCodeKind)
{
    ASSERT(0 <= sourceCodeKind  && sourceCodeKind < registeredSourceCodeKinds.size());
    return registeredSourceCodeKinds[sourceCodeKind].funGetSourceCodeLine;
}

FTranslateCtToRt Nest_getTranslateCtToRtFun(int sourceCodeKind)
{
    ASSERT(0 <= sourceCodeKind  && sourceCodeKind < registeredSourceCodeKinds.size());
    return registeredSourceCodeKinds[sourceCodeKind].funTranslateCtToRt;
}

int Nest_getSourceCodeKindForExtension(const char* extension)
{
    for ( int i=0; i<registeredSourceCodeKinds.size(); ++i )
    {
        if ( 0 == strcmp(extension, registeredSourceCodeKinds[i].extension) )
            return i;
    }
    return -1;
}

int Nest_getSourceCodeKindForFilename(const char* filename)
{
    const string& ext = extensionFromFilename(filename);
    return Nest_getSourceCodeKindForExtension(ext.c_str());
}
