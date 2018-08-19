#include <StdInc.h>

#include "SprDebug.h"
#include "Nest/Api/SourceCode.h"
#include "Nest/Utils/cppif/NodeUtils.hpp"

#include "Nodes/SparrowNodes.h"
#include "Nodes/SparrowNodesAccessors.h"
#include "Helpers/SprTypeTraits.h"

// Defined in SparrowNodes_Module.cpp
StringRef inferModuleName(const char* url);

namespace {

template <typename T> T extractValue(StringRef valueData) {
    ASSERT(valueData.size() == sizeof(T));
    // NOLINTNEXTLINE
    return *reinterpret_cast<const T*>(valueData.begin);
}

void printCtValue(StringRef typeName, StringRef valueDataStr) {
    if (typeName == "Type/ct") {
        auto t = extractValue<TypeRef>(valueDataStr);
        printf("%s", t->description);
    } else if (typeName == "Bool") {
        bool val = 0 != extractValue<uint8_t>(valueDataStr);
        printf("%s", val ? "true" : "false");
    } else if (typeName == "Short")
        printf("%d", (int)extractValue<int16_t>(valueDataStr));
    else if (typeName == "UShort")
        printf("%d", (int)extractValue<uint16_t>(valueDataStr));
    else if (typeName == "Int")
        printf("%d", (int)extractValue<int32_t>(valueDataStr));
    else if (typeName == "UInt")
        printf("%u", (unsigned)extractValue<uint16_t>(valueDataStr));
    else if (typeName == "Long")
        printf("%lld", (long long)extractValue<int64_t>(valueDataStr));
    else if (typeName == "ULong")
        printf("%llu", (unsigned long long)extractValue<uint64_t>(valueDataStr));
    else if (typeName == "Float")
        printf("%g", extractValue<float>(valueDataStr));
    else if (typeName == "Double")
        printf("%g", extractValue<double>(valueDataStr));
    else if (typeName == "StringRef") {
        printf("\"%s\"", valueDataStr.begin);
    } else
        printf("'%s'", valueDataStr.begin);
}
void printCtValueNode(Node* node) {
    TypeRef type = Nest_getCheckPropertyType(node, "valueType");
    StringRef valueDataStr = Nest_getCheckPropertyString(node, "valueData");

    printCtValue(StringRef(type->description), valueDataStr);
}
void printLiteralNode(Node* node) {
    StringRef litType = Nest_getCheckPropertyString(node, "spr.literalType");
    StringRef data = Nest_getCheckPropertyString(node, "spr.literalData");

    printCtValue(litType, data);
}
} // namespace

extern "C" {

static int g_debugIndent = 0;

int atLocation(
        Location loc, const char* filename, int startLine, int startCol, int endLine, int endCol) {
    // Check the URL part
    if (filename && loc.sourceCode) {
        const char* p = strstr(loc.sourceCode->url, filename);
        if (!p)
            return 0;
    }

    // Check the line & cols
    return (startLine == 0 || startLine == int(loc.start.line)) &&
           (startCol == 0 || startCol == int(loc.start.col)) &&
           (endLine == 0 || endLine == int(loc.end.line)) &&
           (endCol == 0 || endCol == int(loc.end.col));
}

void printSpaces() {
    for (int i = 0; i < g_debugIndent; i++)
        printf("  ");
}

void printStart() {
    printSpaces();
    printf("{\n");
    g_debugIndent++;
}
void printEnd() {
    g_debugIndent--;
    printSpaces();
    printf("}\n");
}

void printNodeImpl(Node* node, int mode);

/// Same as printNodeImpl(node, 2), but sometimes add parenthesis around the expression
/// If the node is null, doesn't print anything
void printExpNodeParens(Node* node) {
    if (!node) {
        // nothing
    } else if (node->nodeKind == nkSparrowExpInfixExp) {
        printf("(");
        printNodeImpl(node, 2);
        printf(")");
    } else
        printNodeImpl(node, 2);
}

/// Print the node, in the given mode
///
/// possible modes:
///     - 0 = auto - try to find the best way to print the node
///     - 1 - top-level, as a separate line (don't add any spaces for the first row)
///     - 2 - single line (expression)
void printNodeImpl(Node* node, int mode) {
    if (!node) {
        return;
    }

    // Feather nodes
    switch (node->nodeKind - Feather_getFirstFeatherNodeKind()) {
    case nkRelFeatherNop:
        printf("Nop");
        return;
    case nkRelFeatherTypeNode:
        printf("%s", Nest_getCheckPropertyType(node, "givenType")->description);
        return;
    case nkRelFeatherBackendCode:
        printf("BackendCode");
        return;
    case nkRelFeatherNodeList:
        if (mode == 2) {
            // Print it as an expression
            printf("(");
            for (unsigned i = 0; i < size(node->children); i++) {
                if (i > 0)
                    printf(", ");
                printNodeImpl(at(node->children, i), mode);
            }
            printf(")");
            return;
        }
        // Else fall-through to LocalSpace case
    case nkRelFeatherLocalSpace:
        printf("{\n");
        g_debugIndent++;
        for (Node* child : node->children) {
            printSpaces();
            printNodeImpl(child, 1);
            printf(";\n");
        }
        g_debugIndent--;
        printSpaces();
        printf("}");
        return;
    case nkRelFeatherGlobalConstructAction:
        printf("GlobalDestructAction(");
        printNodeImpl(at(node->children, 0), 2);
        printf(")");
        return;
    case nkRelFeatherGlobalDestructAction:
        printf("GlobalConstructAction(");
        printNodeImpl(at(node->children, 0), 2);
        printf(")");
        return;
    case nkRelFeatherScopeDestructAction:
        printf("GlobalScopeDestructAction(");
        printNodeImpl(at(node->children, 0), 2);
        printf(")");
        return;
    case nkRelFeatherTempDestructAction:
        printf("GlobalTempDestructAction(");
        printNodeImpl(at(node->children, 0), 2);
        printf(")");
        return;
    case nkRelFeatherChangeMode:
        printf("ChangeMode(%d, ", Nest_getCheckPropertyInt(node, propEvalMode));
        printNodeImpl(at(node->children, 0), 2);
        printf(")");
        return;

    case nkRelFeatherDeclFunction: {
        Node* returnType = at(node->children, 0);
        Node* body = at(node->children, 1);
        Nest_NodeRange params = all(node->children);
        params.beginPtr += 2;
        printf("fun %s", Feather_getName(node).begin);
        if (size(params) > 0) {
            printf("(");
            bool first = true;
            for (auto p : params) {
                if (!p)
                    continue;
                if (first)
                    first = false;
                else
                    printf(", ");
                if (p->nodeKind == nkFeatherDeclVar) {
                    Node* typeNode = at(node->children, 0);
                    printf("%s: ", Feather_getName(node).begin);
                    printNodeImpl(typeNode, 2);
                } else
                    printNodeImpl(p, 2);
            }
            printf(")");
        }
        if (returnType) {
            printf(": ");
            printNodeImpl(returnType, 2);
        }
        if (body) {
            printf("\n");
            printSpaces();
            printNodeImpl(body, 1);
        }
        return;
    }
    case nkRelFeatherDeclClass:
        printf("class %s;", Feather_getName(node).begin);
        return;
    case nkRelFeatherDeclVar:
        printf("var %s;", Feather_getName(node).begin);
        return;

    case nkRelFeatherExpCtValue:
        printCtValueNode(node);
        return;
    case nkRelFeatherExpNull:
        printf("null");
        return;
    case nkRelFeatherExpVarRef:
        printf("VarRef(%s)", Feather_getName(at(node->referredNodes, 0)).begin);
        return;
    case nkRelFeatherExpFieldRef:
        printf("FieldRef(");
        printNodeImpl(at(node->children, 0), 2);
        printf(", %s)", Feather_getName(at(node->referredNodes, 0)).begin);
        return;
    case nkRelFeatherExpFunRef:
        printf("FunRef(%s)", Feather_getName(at(node->referredNodes, 0)).begin);
        return;
    case nkRelFeatherExpFunCall:
        printf("%s(", Feather_getName(at(node->referredNodes, 0)).begin);
        for (unsigned i = 0; i < Nest_nodeArraySize(node->children); ++i) {
            if (i != 0)
                printf(", ");
            printNodeImpl(at(node->children, i), 2);
        }
        printf(")");
        return;
    case nkRelFeatherExpMemLoad:
        printf("MemLoad(");
        printNodeImpl(at(node->children, 0), 2);
        printf(")");
        return;
    case nkRelFeatherExpMemStore:
        printf("MemStore(");
        printNodeImpl(at(node->children, 0), 2);
        printf(", ");
        printNodeImpl(at(node->children, 1), 2);
        printf(")");
        return;
    case nkRelFeatherExpBitcast:
        printf("Bitcast(");
        printNodeImpl(at(node->children, 0), 2);
        printf(", ");
        printNodeImpl(at(node->children, 1), 2);
        printf(")");
        return;
    case nkRelFeatherExpConditional:
        printf("ife(");
        printNodeImpl(at(node->children, 0), 2);
        printf(", ");
        printNodeImpl(at(node->children, 1), 2);
        printf(", ");
        printNodeImpl(at(node->children, 2), 2);
        printf(")");
        return;

    case nkRelFeatherStmtIf:
        printf("if ( ");
        printNodeImpl(at(node->children, 0), 2);
        printf(" )\n");
        g_debugIndent++;
        printSpaces();
        printNodeImpl(at(node->children, 1), 1);
        printf(";\n");
        g_debugIndent--;
        printSpaces();
        printf("else\n");
        g_debugIndent++;
        printSpaces();
        printNodeImpl(at(node->children, 2), 1);
        printf(";\n");
        g_debugIndent--;
        printSpaces();
        return;
    case nkRelFeatherStmtWhile:
        printf("while ( ");
        printNodeImpl(at(node->children, 0), 2);
        if (at(node->children, 1)) {
            printf("; ");
            printNodeImpl(at(node->children, 1), 2);
        }
        printf(" )\n");
        g_debugIndent++;
        printSpaces();
        printNodeImpl(at(node->children, 2), 1);
        printf(";\n");
        g_debugIndent--;
        printSpaces();
        return;
    case nkRelFeatherStmtBreak:
        printf("break");
        return;
    case nkRelFeatherStmtContinue:
        printf("continue");
        return;
    case nkRelFeatherStmtReturn:
        printf("return ");
        Node* exp = at(node->children, 0);
        if (exp)
            printNodeImpl(exp, 2);
        return;
    }

    // Sparrow nodes
    switch (node->nodeKind - firstSparrowNodeKind) {
    case nkRelSparrowModifiersNode: {
        printf("[");
        Node* mods = at(node->children, 1);
        bool first = true;
        for (Node* n : mods->children) {
            if (!first)
                printf(", ");
            first = false;
            printNodeImpl(n, 2);
        }
        printf("] ");
        printNodeImpl(at(node->children, 0), mode);
        return;
    }

    // case nkRelSparrowDeclModule:
    //     return;
    case nkRelSparrowDeclImportName: {
        Node* moduleName = at(node->children, 0);
        Node* declNames = at(node->children, 1);
        if (Feather_hasName(node) && StringRef(Feather_getName(node)))
            printf("import %s = ", Feather_getName(node).begin);
        else
            printf("import ");
        printNodeImpl(moduleName, 2);
        if (declNames) {
            printNodeImpl(declNames, 2);
        }
        return;
    }
    case nkRelSparrowDeclPackage:
        printf("package %s\n", Feather_getName(node).begin);
        printSpaces();
        printNodeImpl(at(node->children, 0), 1);
        return;
    case nkRelSparrowDeclSprDatatype: {
        Node* parameters = at(node->children, 0);
        Node* children = at(node->children, 1);
        Node* ifClause = at(node->children, 2);

        printf("class %s", Feather_getName(node).begin);
        printNodeImpl(parameters, 2);
        if (ifClause) {
            printf(" if ");
            printNodeImpl(ifClause, 2);
        }
        printf("\n");
        printSpaces();
        printNodeImpl(children, 1);
        return;
    }
    case nkRelSparrowDeclSprFunction: {
        Node* parameters = at(node->children, 0);
        Node* returnType = at(node->children, 1);
        Node* body = at(node->children, 2);
        Node* ifClause = at(node->children, 3);

        printf("fun %s", Feather_getName(node).begin);
        printNodeImpl(parameters, 2);
        if (returnType) {
            printf(": ");
            printNodeImpl(returnType, 2);
        }
        if (ifClause) {
            printf(" if ");
            printNodeImpl(ifClause, 2);
            printf(";\n");
            printSpaces();
        }
        if (body) {
            printf("\n");
            printSpaces();
            printNodeImpl(body, 1);
        }
        return;
    }
    case nkRelSparrowDeclSprVariable:
        printf("var ");
        // same code as the parameter
    case nkRelSparrowDeclSprParameter: {
        Node* typeNode = at(node->children, 0);
        Node* init = at(node->children, 1);
        printf("%s", Feather_getName(node).begin);
        if (typeNode) {
            printf(": ");
            printNodeImpl(typeNode, 2);
        } else if (Nest_hasProperty(node, "spr.givenType")) {
            TypeRef givenType = Nest_getCheckPropertyType(node, "spr.givenType");
            printf(": %s", givenType->description);
        }
        if (init) {
            printf(" = ");
            printNodeImpl(init, 2);
        }
        return;
    }
    case nkRelSparrowDeclSprConcept: {
        Node* baseConcept = at(node->children, 0);
        Node* ifClause = at(node->children, 1);
        // Node*& instantiationsSet = at(node->children, 2);
        StringRef paramName = Nest_getCheckPropertyString(node, "spr.paramName");
        printf("concept %s(%s", Feather_getName(node).begin, paramName.begin);
        if (baseConcept) {
            printf(": ");
            printNodeImpl(baseConcept, 2);
        }
        printf(")\n");
        g_debugIndent++;
        printSpaces();
        printf("if ");
        printNodeImpl(ifClause, 2);
        g_debugIndent--;
        return;
    }
    // case nkRelSparrowDeclGenericClass:
    //     return;
    // case nkRelSparrowDeclGenericFunction:
    //     return;
    case nkRelSparrowDeclUsing:
        if (Feather_hasName(node))
            printf("using '%s' = ", Feather_getName(node).begin);
        else
            printf("using ");
        printNodeImpl(at(node->children, 0), 2);
        return;

    case nkRelSparrowExpLiteral:
        printLiteralNode(node);
        return;
    case nkRelSparrowExpIdentifier:
        printf("%s", Feather_getName(node).begin);
        return;
    case nkRelSparrowExpCompoundExp:
        printNodeImpl(at(node->children, 0), 2);
        printf(".%s", Feather_getName(node).begin);
        return;
    case nkRelSparrowExpFunApplication: {
        Node* base = at(node->children, 0);
        Node* arguments = at(node->children, 1);
        printNodeImpl(base, 2);
        printNodeImpl(arguments, 2);
        return;
    }
    case nkRelSparrowExpOperatorCall:
        // fall-through
    case nkRelSparrowExpInfixExp: {
        Node* arg1 = at(node->children, 0);
        Node* arg2 = at(node->children, 1);
        StringRef op = Nest_getCheckPropertyString(node, "spr.operation");
        printNodeImpl(arg1, 2);
        printf(" %s ", op.begin);
        printNodeImpl(arg2, 2);
        return;
    }
    // case nkRelSparrowExpLambdaFunction:
    //     return;
    // case nkRelSparrowExpSprConditional:
    //     return;
    case nkRelSparrowExpDeclExp:
        if (size(node->referredNodes) == 2) {
            // Only one referred decl
            Node* decl = at(node->referredNodes, 1);
            TypeRef t = tryGetTypeValue(decl);
            printf("%s", t ? t->description : Feather_getName(decl).begin);
        } else {
            printf("decls(");
            for (int i = 1; i < size(node->referredNodes); i++) {
                if (i > 1)
                    printf(", ");
                Node* decl = at(node->referredNodes, i);
                TypeRef t = tryGetTypeValue(decl);
                printf("%s", t ? t->description : Feather_getName(decl).begin);
            }
            printf(")");
        }
        return;
    case nkRelSparrowExpStarExp:
        printNodeImpl(at(node->children, 0), 2);
        printf(".*");
        return;
    case nkRelSparrowExpModuleRef: {
        Node* moduleNode = at(node->referredNodes, 0);
        Node* moduleName = at(moduleNode->children, 0);
        if (moduleName) {
            printf("%s", Feather_getName(moduleName).begin);
        } else {
            printf("%s", inferModuleName(moduleNode->location.sourceCode->url).begin);
        }
        return;
    }

    case nkRelSparrowStmtFor: {
        Node* range = at(node->children, 0);
        Node* action = at(node->children, 1);
        Node* typeExpr = at(node->children, 2);
        printf("for ( %s", Feather_getName(node).begin);
        if (typeExpr) {
            printf(" : ");
            printNodeImpl(typeExpr, 2);
        }
        printf(" = ");
        printNodeImpl(range, 2);
        printf(")\n");
        g_debugIndent++;
        printSpaces();
        printNodeImpl(action, 1);
        printf(";\n");
        g_debugIndent--;
        printSpaces();
        return;
    }
    case nkRelSparrowStmtSprReturn:
        printf("return ");
        printNodeImpl(at(node->children, 0), 2);
        return;

        // case nkRelSparrowInnerInstantiation:
        //     return;
        // case nkRelSparrowInnerInstantiationsSet:
        //     return;
    }

    // Unknown node
    printf("<%s>", Nest_toStringEx(node));
}

void printNode(Node* node) {
    printSpaces();
    printNodeImpl(node, 1);
    printf("\n");
}

void printNodeExp(Node* node) { printNodeImpl(node, 2); }

void printNodes(Nest_NodeRange nodes) {
    for (auto n : nodes)
        printNode(n);
}
}
