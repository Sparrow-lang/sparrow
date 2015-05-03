#include <StdInc.h>
#include "CompilerInstance.h"
#include "CompilerImpl.h"
#include <Common/Diagnostic.h>

using namespace Nest;

Compiler& Nest::theCompiler()
{
	return CompilerInstance::instance().compiler();
}

CompilerInstance::CompilerInstance()
    : compiler_(nullptr)
{
}

CompilerInstance& CompilerInstance::instance()
{
    static CompilerInstance theInstance;
    return theInstance;
}

Compiler& CompilerInstance::compiler() const
{
    return *compiler_;
}


void CompilerInstance::init()
{
    ASSERT(compiler_ == nullptr);
    compiler_ = new CompilerImpl();
}

void CompilerInstance::destroy()
{
    delete compiler_;
    compiler_ = nullptr;
}

void CompilerInstance::reset()
{
    destroy();
    init();
}
