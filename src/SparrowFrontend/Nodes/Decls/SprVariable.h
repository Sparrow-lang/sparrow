#pragma once

#include "AccessType.h"
#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// Class that represents a Sparrow variable declaration (global var, local var or attribute)
    class SprVariable : public DynNode
    {
        DEFINE_NODE(SprVariable, nkSparrowDeclSprVariable, "Sparrow.Decl.Variable");

    public:
        SprVariable(const Location& loc, string name, DynNode* typeNode, DynNode* init, AccessType accessType = publicAccess);
        SprVariable(const Location& loc, string name, Nest::TypeRef type, DynNode* init, AccessType accessType = publicAccess);

        virtual void dump(ostream& os) const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();
    };
}
