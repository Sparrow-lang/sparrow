#include <StdInc.h>
#include "ModifiersNode.h"
#include "SprProperties.h"
#include "SparrowNodesAccessors.h"
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

#include <Feather/Util/Decl.h>


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
    
    if ( modNode->nodeKind == nkSparrowExpIdentifier )
    {
        const string& id = Feather::getName(modNode);
        if ( id == "static" )
            mod = new ModStatic;
        else if ( id == "ct" )
            mod = new ModCt;
        else if ( id == "rt" )
            mod = new ModRt;
        else if ( id == "rtct" )
            mod = new ModRtCt;
        else if ( id == "autoCt" )
            mod = new ModAutoCt;
        else if ( id == "ctGeneric" )
            mod = new ModCtGeneric;
        else if ( id == "convert" )
            mod = new ModConvert;
        else if ( id == "noDefault" )
            mod = new ModNoDefault;
        else if ( id == "initCtor" )
            mod = new ModInitCtor;
        else if ( id == "macro" )
            mod = new ModMacro;
        else if ( id == "noInline" )
            mod = new ModNoInline;
    }
    else
    {
        if ( modNode->nodeKind == nkSparrowExpInfixExp )
        {
            Node* ident = ofKind(modNode->children[0], nkSparrowExpIdentifier);
            Node* args = modNode->children[1];
            if ( args && args->nodeKind != Feather::nkFeatherNodeList )
                args = nullptr;
            if ( ident && args && args->children.size() == 1 )
            {
                Node* funArg = ofKind(args->children.front(), nkSparrowExpLiteral);

                if ( Feather::getName(ident) == "native" && funArg && Literal_isString(funArg) )
                    mod = new ModNative(Literal_getData(funArg));
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

