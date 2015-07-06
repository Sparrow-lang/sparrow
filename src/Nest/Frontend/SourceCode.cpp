#include <StdInc.h>
#include "SourceCode.h"

using namespace Nest;

SourceCode::SourceCode(const string& filename)
    : filename_(filename)
    , iCode_(nullptr)
{
}

SourceCode::~SourceCode()
{
}

boost::function<DynNode*(DynNode*)> SourceCode::ctToRtTranslator() const
{
    return boost::function<DynNode*(DynNode*)>();
}
