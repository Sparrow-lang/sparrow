#include <StdInc.h>
#include "ModifiersNode.h"
#include "SprProperties.h"
#include "Exp/Identifier.h"
#include "Exp/Literal.h"
#include "Exp/FunApplication.h"
#include "Exp/InfixExp.h"
#include <Helpers/ForEachNodeInNodeList.h>


#include "Mods/ModStatic.h"
#include "Mods/ModCt.h"
#include "Mods/ModRt.h"
#include "Mods/ModRtCt.h"
#include "Mods/ModAutoCt.h"
#include "Mods/ModCtGeneric.h"
#include "Mods/ModNative.h"
#include "Mods/ModConvert.h"
#include "Mods/ModNoDefault.h"
#include "Mods/ModInitCtor.h"
#include "Mods/ModMacro.h"
#include "Mods/ModNoInline.h"


ModifiersNode::ModifiersNode(const Location& loc, Node* base, Node* modifierNodes)
    : DynNode(classNodeKind(), loc, {base, modifierNodes})
{
}

void ModifiersNode::doSetContextForChildren()
{
    Node* base = data_.children[0];
    Node* modifierNodes = data_.children[1];

    // Set the context of the modifiers
    if ( modifierNodes )
        Nest::setContext(modifierNodes, data_.context);

    // Interpret the modifiers here - as much as possible
    interpretModifiers();

    // Set the context for the base
    if ( base )
        Nest::setContext(base, data_.context);
}

void ModifiersNode::doComputeType()
{
    Node* base = data_.children[0];

    // Compute the type of the base node
    Nest::computeType(base);
    data_.type = base->type;
    data_.explanation = base;
}

void ModifiersNode::doSemanticCheck()
{
    Nest::computeType(node());

    // Semantic check the base
    Node* base = data_.children[0];
    Nest::semanticCheck(base);
}

void ModifiersNode::interpretModifiers()
{
    Node* modifierNodes = data_.children[1];

    if ( modifierNodes )
    {
        if ( modifierNodes->nodeKind == Feather::nkFeatherNodeList )
        {
            // Add the modifiers to the base node
            forEachNodeInNodeList(modifierNodes, [&] (Node* modNode)
            {
                applyModifier(modNode);
            });
        }
        else
        {
            applyModifier(modifierNodes);
        }
    }
}

void ModifiersNode::applyModifier(Node* modNode)
{
    Nest::Modifier* mod = nullptr;
    
    Identifier* ident = (Identifier*) ofKind(modNode, nkSparrowExpIdentifier);
    if ( ident )
    {
        if ( ident->id() == "static" )
            mod = new ModStatic;
        else if ( ident->id() == "ct" )
            mod = new ModCt;
        else if ( ident->id() == "rt" )
            mod = new ModRt;
        else if ( ident->id() == "rtct" )
            mod = new ModRtCt;
        else if ( ident->id() == "autoCt" )
            mod = new ModAutoCt;
        else if ( ident->id() == "ctGeneric" )
            mod = new ModCtGeneric;
        else if ( ident->id() == "convert" )
            mod = new ModConvert;
        else if ( ident->id() == "noDefault" )
            mod = new ModNoDefault;
        else if ( ident->id() == "initCtor" )
            mod = new ModInitCtor;
        else if ( ident->id() == "macro" )
            mod = new ModMacro;
        else if ( ident->id() == "noInline" )
            mod = new ModNoInline;
    }
    else
    {
        InfixExp* fapp = (InfixExp*) ofKind(modNode, nkSparrowExpInfixExp);
        if ( fapp )
        {
            Identifier* ident = nullptr;
            if ( fapp->arg1()->nodeKind == nkSparrowExpIdentifier )
                ident = (Identifier*) fapp->arg1();
            Node* args = fapp->arg2();
            if ( args && args->nodeKind != Feather::nkFeatherNodeList )
                args = nullptr;
            if ( ident && args && args->children.size() == 1 )
            {
                Literal* funArg = nullptr;
                if ( args->children.front()->nodeKind == nkSparrowExpLiteral )
                    funArg = (Literal*) args->children.front();

                if ( ident->id() == "native" && funArg && funArg->isString() )
                    mod = new ModNative(funArg->asString());
            }
        }
    }
    
    // If we recognized a modifier, add it to the base node; otherwise raise an error
    Node* base = data_.children[0];
    if ( mod )
        Nest::addModifier(base, mod);
    else
        REP_ERROR(modNode->location, "Unknown modifier found (%1%)") % modNode;
}

