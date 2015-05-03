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

boost::function<Node*(Node*)> SourceCode::ctToRtTranslator() const
{
    return boost::function<Node*(Node*)>();
}
