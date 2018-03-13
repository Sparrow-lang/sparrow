#include <StdInc.h>
#include "AstDump.h"

#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Utils/NodeUtils.hpp"
#include "Nest/Api/Node.h"
#include "Nest/Api/NodeArray.h"
#include "Nest/Api/Type.h"

struct JsonContext {
    FILE* f;
    int indent;
    bool insideArray;
    bool firstAttribute;
    bool oneLine;

    const SourceCode* sourceCodeFilter;

    std::unordered_set<void*> writtenPointers;
};

bool _satisfiesFilter(JsonContext* ctx, Node* node) {
    Location loc = node->location;
    if (loc.sourceCode != ctx->sourceCodeFilter)
        return false;
    return true;
}

void _writeSpaces(JsonContext* ctx) {
    static const int indentSize = 2;
    static const int spacesLen = 20;
    char spaces[spacesLen + 1] = "                    ";
    int toWrite = indentSize * ctx->indent;
    // Write in big chunks
    while (toWrite > spacesLen) {
        fwrite(spaces, 1, spacesLen, ctx->f);
        toWrite -= spacesLen;
    }
    // Write the remaining of spaces
    if (toWrite > 0)
        fwrite(spaces, 1, toWrite, ctx->f);
}

void _writePrefix(JsonContext* ctx, const char* prefix) {
    if (ctx->firstAttribute)
        ctx->firstAttribute = false;
    else if (ctx->oneLine) {
        fputs(", ", ctx->f);
    } else {
        fputc(',', ctx->f);
    }

    if (!ctx->oneLine) {
        fputc('\n', ctx->f);
        _writeSpaces(ctx);
    }

    if (!ctx->insideArray) {
        fputs("\"", ctx->f);
        fputs(prefix, ctx->f);
        fputs("\": ", ctx->f);
    }
}

void _writeNumber(JsonContext* ctx, const char* name, long long val) {
    _writePrefix(ctx, name);
    fprintf(ctx->f, "%lld", val);
}
void _writeFloatingNumber(JsonContext* ctx, const char* name, double val) {
    _writePrefix(ctx, name);
    fprintf(ctx->f, "%f", val);
}
void _writeStringL(JsonContext* ctx, const char* name, const char* val, size_t size) {
    // Check if the string contains some 'strange' characters
    bool hasStrangeCharacters = false;
    for (size_t i = 0; i < size; ++i) {
        char ch = val[i];
        if (!isprint(ch) || ch == '\\' || ch == '\"') {
            hasStrangeCharacters = true;
            break;
        }
    }

    _writePrefix(ctx, name);
    fputc('"', ctx->f);
    if (!hasStrangeCharacters)
        fputs(val, ctx->f);
    else {
        for (size_t i = 0; i < size; ++i) {
            unsigned char ch = val[i];
            if (ch == '\\')
                fputs("\\\\", ctx->f);
            else if (ch == '"')
                fputs("\\\"", ctx->f);
            else if (isprint(ch))
                fputc(ch, ctx->f);
            else {
                unsigned char hi = (ch & 0xF0) >> 4;
                unsigned char lo = ch & 0x0F;
                static const char* hexChars = "0123456789ABCDEF";
                char hex[6] = {'\\', 'u', '0', '0', hexChars[hi], hexChars[lo]};
                fwrite(hex, 1, 6, ctx->f);
            }
        }
    }
    fputc('"', ctx->f);
}
void _writeString(JsonContext* ctx, const char* name, const char* val) {
    _writeStringL(ctx, name, val, strlen(val));
}
void _writeStringSR(JsonContext* ctx, const char* name, StringRef val) {
    _writeStringL(ctx, name, val.begin, val.end - val.begin);
}

void _startObject(JsonContext* ctx, const char* name) {
    _writePrefix(ctx, name);
    fputs("{", ctx->f);
    ++ctx->indent;
    ctx->firstAttribute = true;
    ctx->insideArray = false;
}

void _endObject(JsonContext* ctx) {
    --ctx->indent;
    if (!ctx->oneLine) {
        fputc('\n', ctx->f);
        _writeSpaces(ctx);
    }
    fputc('}', ctx->f);

    ctx->firstAttribute = false;
}

void _startArray(JsonContext* ctx, const char* name) {
    _writePrefix(ctx, name);
    fputc('[', ctx->f);
    ++ctx->indent;
    ctx->firstAttribute = true;
    ctx->insideArray = true;
}
void _endArray(JsonContext* ctx) {
    if (!ctx->oneLine)
        fputc('\n', ctx->f);
    --ctx->indent;
    _writeSpaces(ctx);
    fputc(']', ctx->f);
    ctx->firstAttribute = false;
    ctx->insideArray = false;
}

void _writeType(JsonContext* ctx, const char* name, TypeRef t) {
    _startObject(ctx, name);

    size_t ref = reinterpret_cast<size_t>((void*)t);
    _writeNumber(ctx, "ref", ref);
    if (t)
        _writeString(ctx, "desc", t->description);
    _endObject(ctx);
}

void _writeNode(JsonContext* ctx, const char* name, Node* node);

void _writeNodeData(JsonContext* ctx, Node* node) {
    // Node kind
    _writeString(ctx, "kind", Nest_nodeKindName(node));

    // Location
    _startObject(ctx, "location");
    ctx->oneLine = true;
    _startObject(ctx, "start");
    _writeNumber(ctx, "line", node->location.start.line);
    _writeNumber(ctx, "col", node->location.start.col);
    _endObject(ctx);
    _startObject(ctx, "end");
    _writeNumber(ctx, "line", node->location.end.line);
    _writeNumber(ctx, "col", node->location.end.col);
    _endObject(ctx);
    _endObject(ctx);
    ctx->oneLine = false;

    // Write the properties
    if (node->properties.end != node->properties.begin) {
        _startArray(ctx, "properties");
        NodeProperty* p = node->properties.begin;
        for (; p != node->properties.end; ++p) {
            ctx->insideArray = true;
            _startObject(ctx, "");
            ctx->oneLine = true;
            _writeStringSR(ctx, "name", p->name);
            _writeNumber(ctx, "kind", (int)p->kind);
            if (p->passToExpl)
                _writeNumber(ctx, "passToExpl", 1);
            switch (p->kind) {
            case propInt:
                _writeNumber(ctx, "val", p->value.intValue);
                break;
            case propString:
                _writeStringSR(ctx, "val", p->value.stringValue);
                break;
            case propNode:
                ctx->oneLine = false;
                _writeNode(ctx, "val", p->value.nodeValue);
                break;
            case propType:
                _writeType(ctx, "val", p->value.typeValue);
                break;
            }
            _endObject(ctx);
            ctx->oneLine = false;
        }
        _endArray(ctx);
    }

    // Write node type
    if (node->type)
        _writeType(ctx, "type", node->type);

    // Write its children
    if (size(node->children) > 0) {
        _startArray(ctx, "children");
        for (Node* n : node->children) {
            ctx->insideArray = true;
            _writeNode(ctx, "", n);
        }
        _endArray(ctx);
    }

    // Write the referred nodes
    if (size(node->referredNodes) > 0) {
        _startArray(ctx, "referredNodes");
        for (Node* n : node->referredNodes) {
            ctx->insideArray = true;
            _writeNode(ctx, "", n);
        }
        _endArray(ctx);
    }

    // Write the explanation of the node
    if (node->explanation && node->explanation != node)
        _writeNode(ctx, "explanation", node->explanation);
}

void _writeNode(JsonContext* ctx, const char* name, Node* node) {
    _startObject(ctx, name);

    if (!node || !_satisfiesFilter(ctx, node)) {
        // Null pointer (or not satisfies the filter)
        ctx->oneLine = true;
        _writeNumber(ctx, "ref", 0);
    } else {
        size_t ref = reinterpret_cast<size_t>((void*)node);
        auto it = ctx->writtenPointers.find((void*)node);
        if (it == ctx->writtenPointers.end()) {
            // First time we see this pointer
            ctx->writtenPointers.insert((void*)node);
            _writeNumber(ctx, "ref", ref);
            _startObject(ctx, "obj");
            _writeNodeData(ctx, node);
            _endObject(ctx);
        } else {
            // The object was already serialized
            ctx->oneLine = true;
            _writeNumber(ctx, "ref", ref);
        }
    }

    _endObject(ctx);
    ctx->oneLine = false;
}

void dumpAstNode(Node* node, const char* filename) {
    ASSERT(node);

    cout << "Dumping AST nodes to: " << filename << endl;
    FILE* f = fopen(filename, "w");
    if (!f) {
        Nest_reportFmt(node->location, diagWarning, "Cannot open %s to dump AST node", filename);
        return;
    }

    JsonContext ctx = {f, 0};
    ctx.firstAttribute = true;
    ctx.insideArray = true;
    ctx.sourceCodeFilter = node->location.sourceCode;

    _writeNode(&ctx, "", node);

    fclose(f);
}
