#include <StdInc.h>

#include "Nest/Api/SourceCode.h"
#include "Nest/Utils/NodeUtils.hpp"

#include "Nodes/SparrowNodes.h"
#include "Nodes/SparrowNodesAccessors.h"

// Defined in SparrowNodes_Module.cpp
StringRef inferModuleName(Node* node);

static int g_debugIndent = 0;

bool atLocation(Location loc, const char* filename, int startLine = 0, int startCol = 0, int endLine = 0, int endCol = 0) {
    // Check the URL part
    if ( filename ) {
        const char* url = loc.sourceCode->url;
        auto urlLen = strlen(url);
        auto filenameLen = strlen(filename);
        if ( urlLen < filenameLen )
            return false;
        url += urlLen-filenameLen;
        if ( 0 != strcmp(url, filename) )
            return false;
    }

    // Check the line & cols
    return (startLine == 0 || startLine == loc.start.line)
        && (startCol == 0 || startCol == loc.start.col)
        && (endLine == 0 || endLine == loc.end.line)
        && (endCol == 0 || endCol == loc.end.col)
        ;
}


void printSpaces(bool continueLine = false) {
    if ( !continueLine ) {
        for ( int i=0; i<g_debugIndent; i++ )
            printf("  ");
    }
}

void printStart(const char* job, const char* url, bool continueLine = false) {
    printSpaces(continueLine);
    printf("%s: %s\n", job, url);
    printSpaces();
    printf("{\n");
    g_debugIndent++;
}
void printEnd() {
    g_debugIndent--;
    printSpaces();
    printf("}\n");
}
void printExpNode(Node* node, bool printUnknown = true) {
    // Don't print start or end of the line
    if ( node->nodeKind == nkSparrowExpModuleRef ) {
        Node* moduleNode = at(node->referredNodes, 0);
        Node* moduleName = at(moduleNode->children, 0);
        if ( moduleName ) {
            printf("%s", Feather_getName(moduleName).begin);
        }
        else {
            printf("%s", inferModuleName(moduleNode).begin);
        }
    }
    else if ( node->nodeKind == nkSparrowExpStarExp ) {
        printExpNode(at(node->children, 0), printUnknown);
        printf(".*");
    }
    else if ( node->nodeKind == nkSparrowExpIdentifier ) {
        printf("%s", Feather_getName(node).begin);
    }
    else if ( node->nodeKind == nkSparrowExpCompoundExp ) {
        printExpNode(at(node->children, 0), printUnknown);
        printf(".%s", Feather_getName(node).begin);
    }
    else if ( node->nodeKind == nkSparrowExpDeclExp ) {
        if ( size(node->referredNodes) == 2 ) {
            // Only one referred decl
            printExpNode(at(node->referredNodes, 1), printUnknown);    // 0 is baseExp
        }
        else {
            printf("decls(");
            for ( int i=1; i<size(node->referredNodes); i++ ) {
                if ( i>1 ) printf(", ");
                printExpNode(at(node->referredNodes, i), printUnknown);
            }
            printf(")");
        }
    }
    else if ( node->nodeKind == nkFeatherBackendCode ) {
        printf("<backendCode>");
    }
    else {
        if ( printUnknown )
            printf("<%s>", Nest_toStringEx(node));
        else
            printf("?");
    }
}

void printParams(Node* params) {
    if ( params ) {
        printf("(");
        bool first = true;
        for ( Node* p : params->children ) {
            if ( !first ) printf(", ");
            first = false;
            if ( p->nodeKind == nkSparrowDeclSprParameter ) {
                printf("%s: ", Feather_getName(p).begin);
                Node* typeNode = at(p->children, 0);
                Node* init = at(p->children, 1);
                printExpNode(typeNode, false);
                if ( init ) {
                    printf(" = ");
                    printExpNode(init, false);
                }
            }
            else {
                printExpNode(p);
            }
        }
        printf(")");
    }
}

void printNodeStructure(Node* node, bool continueLine = false) {
    if ( !node ) {
        printSpaces(continueLine);
        printf("<null>\n");
        return;
    }
    else if ( node->nodeKind == nkSparrowDeclPackage ) {
        printStart("package", Feather_getName(node).begin, continueLine);
        printSpaces();
        printf("// Names: ");
        if ( node->childrenContext )
            Nest_dumpSymTabNames(node->childrenContext->currentSymTab);
        else
            printf("<no context set yet>");
        printf("\n");
        printNodeStructure(at(node->children, 0));
        printEnd();
    }
    else if ( node->nodeKind == nkFeatherNodeList ) {
        bool contLine = continueLine;
        for ( Node* child: node->children) {
            printNodeStructure(child, contLine);
            contLine = false;
        }
    }
    else if ( node->nodeKind == nkSparrowDeclUsing ) {
        printSpaces(continueLine);
        if ( Feather_hasName(node) )
            printf("using '%s' = ", Feather_getName(node).begin);
        else
            printf("using ");
        printExpNode(at(node->children, 0));
        printf(";\n");
    }
    else if ( node->nodeKind == nkSparrowDeclSprClass ) {
        Node* parameters = at(node->children, 0);
        Node* ifClause = at(node->children, 2);

        printSpaces(continueLine);
        printf("class %s", Feather_getName(node).begin);
        printParams(parameters);
        if ( ifClause ) {
            printf(" if ");
            printExpNode(ifClause, false);
        }
        printf(";\n");
    }
    else if ( node->nodeKind == nkSparrowDeclSprFunction ) {
        Node* parameters = at(node->children, 0);
        Node* returnType = at(node->children, 1);
        Node* ifClause = at(node->children, 3);

        printSpaces(continueLine);
        printf("fun %s", Feather_getName(node).begin);
        printParams(parameters);
        if ( returnType && returnType->type ) {
            printf(": %s", returnType->type->description);
        }
        if ( ifClause ) {
            printf(" if ");
            printExpNode(ifClause, false);
        }
        printf(";\n");
    }
    else if ( node->nodeKind == nkSparrowModifiersNode ) {
        printSpaces(continueLine);
        printf("[");
        Node* mods = at(node->children, 1);
        bool first = true;
        for ( Node* n : mods->children ) {
            if ( !first ) printf(", ");
            first = false;
            printExpNode(n, false);
        }
        printf("] ");
        printNodeStructure(at(node->children, 0), true);
    }
    else {
        printSpaces(continueLine);
        printf("? %s\n", Nest_toStringEx(node));
    }
}

