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
        nkSparrowDeclFriend,
        nkSparrowDeclUsing,
        
        nkSparrowExpLiteral,
        nkSparrowExpThis,
        nkSparrowExpIdentifier,
        nkSparrowExpCompoundExp,
        nkSparrowExpSprVarRef,
        nkSparrowExpFunApplication,
        nkSparrowExpOperatorCall,
        nkSparrowExpInfixExp,
        nkSparrowExpLambdaClosure,
        nkSparrowExpLambdaFunction,
        nkSparrowExpSprConditional,
        nkSparrowExpDeclExp,
        nkSparrowExpStarExp,
        
        nkSparrowStmtFor,
        nkSparrowStmtSprReturn,
        
        nextNodeKindValue,
    };
}
