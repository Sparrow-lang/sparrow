#pragma once

#include "AccessType.h"
#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// Class that represents a Sparrow package declaration (or compilation unit) - similar to C++'s namespaces
    class Package : public DynNode
    {
        DEFINE_NODE(Package, nkSparrowDeclPackage, "Sparrow.Decl.Package");

    public:
        Package(const Location& loc, string name, NodeList* children, AccessType accessType = publicAccess);

    protected:
        void doSetContextForChildren();
        void doComputeType();
        void doSemanticCheck();
    };
}
