#include <StdInc.h>
#include "ModNative.h"

//#include <Nest/Common/Diagnostic.h>

using namespace SprFrontend;

ModNative::ModNative(const string& name)
    : name_(name)
{
}

void ModNative::beforeComputeType(Node* node)
{
    node->setProperty(Feather::propNativeName, name_);
}
