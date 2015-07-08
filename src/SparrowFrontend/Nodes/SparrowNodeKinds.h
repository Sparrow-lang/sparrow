#pragma once

#include <Feather/Nodes/FeatherNodeKinds.h>

namespace SprFrontend
{
    enum SparrowNodeKinds
    {
        nkSparrowModifiersNode = Feather::nextNodeKindValue,
        
        nkSparrowDeclSprCompilationUnit,
        nkSparrowDeclPackage,
        nkSparrowDeclSprClass,
        nkSparrowDeclSprFunction,
        nkSparrowDeclSprParameter,
        nkSparrowDeclSprVariable,
        nkSparrowDeclSprConcept,
        nkSparrowDeclGenericClass,
        nkSparrowDeclGenericFunction,
        nkSparrowDeclUsing,

        nkSparrowExpLiteral,
        nkSparrowExpThis,
        nkSparrowExpIdentifier,
        nkSparrowExpCompoundExp,
        nkSparrowExpFunApplication,
        nkSparrowExpOperatorCall,
        nkSparrowExpInfixExp,
        nkSparrowExpLambdaFunction,
        nkSparrowExpSprConditional,
        nkSparrowExpDeclExp,
        nkSparrowExpStarExp,
        
        nkSparrowStmtFor,
        nkSparrowStmtSprReturn,
        
        nkSparrowInnerInstantiation,
        nkSparrowInnerInstantiationsSet,

        nextNodeKindValue,
    };
}
