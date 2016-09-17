#include "Nest/Api/SourceCodeKindRegistrar.h"
#include "Nest/Api/StringRef.h"
#include "Nest/Utils/Assert.h"

#include <string.h>

const char* extensionFromFilename(const char* filename)
{
    int len = strlen(filename);
    if ( len < 3 )
        return filename;
    const char* p = filename+(len-1);
    while ( p != filename && *p != '.' )
        --p;
    return p;
}

struct _SourceCodeKindDescription
{
    const char* extension;
    const char* description;
    const char* extraInfo;
    FParseSourceCode funParseSourceCode;
    FGetSourceCodeLine funGetSourceCodeLine;
    FTranslateCtToRt funTranslateCtToRt;
};

/// The registered SourceCode kinds
static const int _maxSourceCodeKinds = 100;
struct _SourceCodeKindDescription _allSourceCodeKinds[100];
unsigned int _numSourceCodeKinds = 0;

int Nest_registerSourceCodeKind(
        const char* extension,
        const char* description,
        const char* extraInfo,
        FParseSourceCode funParseSourceCode,
        FGetSourceCodeLine funGetSourceCodeLine,
        FTranslateCtToRt funTranslateCtToRt)
{
    int kind = _numSourceCodeKinds++;
    struct _SourceCodeKindDescription desc = { extension, description, extraInfo, funParseSourceCode, funGetSourceCodeLine, funTranslateCtToRt };
    _allSourceCodeKinds[kind] = desc;
    return kind;
}

const char* Nest_getSourceCodeExtension(int sourceCodeKind)
{
    ASSERT(0 <= sourceCodeKind  && sourceCodeKind < _numSourceCodeKinds);
    return _allSourceCodeKinds[sourceCodeKind].extension;
}

const char* Nest_getSourceCodeDescription(int sourceCodeKind)
{
    ASSERT(0 <= sourceCodeKind  && sourceCodeKind < _numSourceCodeKinds);
    return _allSourceCodeKinds[sourceCodeKind].description;
}

const char* Nest_getSourceCodeExtraInfo(int sourceCodeKind)
{
    ASSERT(0 <= sourceCodeKind  && sourceCodeKind < _numSourceCodeKinds);
    return _allSourceCodeKinds[sourceCodeKind].extraInfo;
}

FParseSourceCode Nest_getParseSourceCodeFun(int sourceCodeKind)
{
    ASSERT(0 <= sourceCodeKind  && sourceCodeKind < _numSourceCodeKinds);
    return _allSourceCodeKinds[sourceCodeKind].funParseSourceCode;
}

FGetSourceCodeLine Nest_getGetSourceCodeLineFun(int sourceCodeKind)
{
    ASSERT(0 <= sourceCodeKind  && sourceCodeKind < _numSourceCodeKinds);
    return _allSourceCodeKinds[sourceCodeKind].funGetSourceCodeLine;
}

FTranslateCtToRt Nest_getTranslateCtToRtFun(int sourceCodeKind)
{
    ASSERT(0 <= sourceCodeKind  && sourceCodeKind < _numSourceCodeKinds);
    return _allSourceCodeKinds[sourceCodeKind].funTranslateCtToRt;
}

int Nest_getSourceCodeKindForExtension(const char* extension)
{
    int i;
    for ( i=0; i<_numSourceCodeKinds; ++i )
    {
        if ( 0 == strcmp(extension, _allSourceCodeKinds[i].extension) )
            return i;
    }
    return -1;
}

int Nest_getSourceCodeKindForFilename(const char* filename)
{
    const char* ext = extensionFromFilename(filename);
    return Nest_getSourceCodeKindForExtension(ext);
}
