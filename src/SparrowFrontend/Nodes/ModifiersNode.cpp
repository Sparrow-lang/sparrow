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


ModifiersNode::ModifiersNode(const Location& loc, DynNode* base, DynNode* modifierNodes)
    : DynNode(classNodeKind(), loc, {base, modifierNodes})
{
}

void ModifiersNode::doSetContextForChildren()
{
    DynNode* base = (DynNode*) data_.children[0];
    DynNode* modifierNodes = (DynNode*) data_.children[1];

    // Set the context of the modifiers
    if ( modifierNodes )
        modifierNodes->setContext(data_.context);

    // Interpret the modifiers here - as much as possible
    interpretModifiers();

    // Set the context for the base
    if ( base )
        base->setContext(data_.context);
}

void ModifiersNode::doComputeType()
{
    DynNode* base = (DynNode*) data_.children[0];

    // Compute the type of the base node
    base->computeType();
    data_.type = base->type();
    data_.explanation = base->node();
}

void ModifiersNode::doSemanticCheck()
{
    computeType();

    // Semantic check the base
    DynNode* base = (DynNode*) data_.children[0];
    base->semanticCheck();
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
    
    Identifier* ident = ((DynNode*) modNode)->as<Identifier>();
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
        InfixExp* fapp = ((DynNode*) modNode)->as<InfixExp>();
        if ( fapp )
        {
            Identifier* ident = fapp->arg1()->as<Identifier>();
            Node* args = fapp->arg2()->node();
            if ( args && args->nodeKind != Feather::nkFeatherNodeList )
                args = nullptr;
            if ( ident && args && args->children.size() == 1 )
            {
                Literal* funArg = ((DynNode*) args->children.front())->as<Literal>();

                if ( ident->id() == "native" && funArg && funArg->isString() )
                    mod = new ModNative(funArg->asString());
            }
        }
    }
    
    // If we recognized a modifier, add it to the base node; otherwise raise an error
    DynNode* base = (DynNode*) data_.children[0];
    if ( mod )
        base->addModifier(mod);
    else
        REP_ERROR(modNode->location, "Unknown modifier found (%1%)") % modNode;
}

