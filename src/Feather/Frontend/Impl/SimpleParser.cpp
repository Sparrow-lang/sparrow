#include <StdInc.h>
#include "SimpleParser.h"
#include "SimpleLexer.h"
#include "SimpleAstNode.h"

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/Exp/AtomicOrdering.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Decl.h>
#include <Feather/FeatherTypes.h>

#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Api/Node.h"
#include "Nest/Api/SymTab.h"
#include "Nest/Api/CompilationContext.h"

#include <boost/algorithm/string/case_conv.hpp>


using namespace Feather;
using namespace Nest;

namespace
{
    unsigned char decodeDigit(char digit)
    {
        if ( digit >= 'a' )
            return digit - 'a' + 10;
        else if ( digit >= 'A' )
            return digit - 'A' + 10;
        else
            return digit - '0';
    }
    
    unsigned char decodeByte(const char* twoDigits)
    {
        unsigned char firstPart = decodeDigit(*twoDigits++);
        unsigned char secondPart = decodeDigit(*twoDigits);
        return (firstPart << 4) + secondPart;
    }
    
    string decodeBinaryValue(const Location& loc, const string& src)
    {
        // Remove all the spaces
        string normalized = boost::algorithm::replace_all_copy(src, " ", "");
        
        // Check that the length of the value to be even
        if ( normalized.size() % 2 == 1 )
            REP_ERROR_RET(string(), loc, "Invalid binary value");
        
        // Decode the bytes
        string res;
        res.resize(normalized.size()/2, (char) 0);
        for ( size_t idx=0; idx < normalized.size(); idx += 2 )
        {
            res[idx/2] = decodeByte(&normalized[idx]);
        }
        return res;
    }
}

namespace
{
    Node* interpretNode(CompilationContext* context, SimpleAstNode* srcNode);

    void checkNoChildren(SimpleAstNode* srcNode)
    {
        if ( !srcNode->children().empty() )
        {
            REP_ERROR(srcNode->location(), "%1% shouldn't have any parameters") % srcNode->stringValue();
        }
    }

    void checkChildrenCount(SimpleAstNode* srcNode, size_t expectedCount, const char* paramsDescription)
    {
        if ( srcNode->children().size() != expectedCount )
        {
            REP_ERROR(srcNode->location(), "Expected %1% parameters for %2% (%3%)")
                % expectedCount % srcNode->stringValue() % paramsDescription;
        }
    }

    void checkChildrenCountRange(SimpleAstNode* srcNode, size_t expectedMin, size_t expectedMax, const char* paramsDescription)
    {
        if ( srcNode->children().size() < expectedMin || expectedMax < srcNode->children().size() )
        {
            REP_ERROR(srcNode->location(), "Expected %1%-%2% parameters for %3% (%4%)")
                % expectedMin % expectedMax % srcNode->stringValue() % paramsDescription;
        }
    }

    // Find a definition in the current symbol table, based on the name
    Node* findDefinition(int nodeKind, SymTab* symTab, const string& name, const Location& loc, const char* desc, bool onlyCurrent = false)
    {
        ASSERT(symTab);
        NodeArray entries = onlyCurrent ? Nest_symTabLookupCurrent(symTab, name.c_str()) : Nest_symTabLookup(symTab, name.c_str());
        auto numEntries = Nest_nodeArraySize(entries);
        if ( numEntries == 0 )
        {
            REP_ERROR(loc, "Cannot find %1%") % name;
            return nullptr;
        }
        if ( numEntries > 1 )
        {
            REP_ERROR(loc, "Multiple definitions found for name %1%") % name;
            for ( Node* entry: entries )
            {
                if ( entry )
                    REP_INFO(entry->location, "See possible alternative");
            }
            Nest_freeNodeArray(entries);
            return nullptr;
        }
        Node* res = at(entries, 0);
        Nest_freeNodeArray(entries);
        if ( res->nodeKind != nodeKind  )
        {
            REP_ERROR(loc, "Identifier %1% doesn't denote a %2% (we have %3%)") % name % desc % res;
            return nullptr;
        }
        return res;
    }
    Node* findDefinition(int nodeKind, CompilationContext* context, const string& name, const Location& loc, const char* desc)
    {
        return findDefinition(nodeKind, context->currentSymTab, name, loc, desc);
    }

    string readIdentifier(SimpleAstNode* srcNode, const char* errDetails)
    {
        if ( !srcNode->isIdentifier() )
        {
            REP_ERROR(srcNode->location(), "Expecting identifier (%1%)") % errDetails;
            return string();
        }
        return srcNode->stringValue();
    }

    string readString(SimpleAstNode* literalNode, const char* errDetails)
    {
        if ( !literalNode->isStringConstant() )
        {
            REP_ERROR(literalNode->location(), "Expecting string constant (%1%)") % errDetails;
            return string();
        }
        return literalNode->stringValue();
    }

    int readInt(SimpleAstNode* intNode, const char* errDetails)
    {
        if ( !intNode->isIntConstant() )
        {
            REP_ERROR(intNode->location(), "Expecting integer constant (%1%)") % errDetails;
            return 0;
        }
        return intNode->intValue();
    }

    Node* readNode(CompilationContext* context, SimpleAstNode* srcNode, const char* errDetails)
    {
        if ( !srcNode->isIdentifier() )
        {
            REP_ERROR(srcNode->location(), "Expecting node argument (%1%)") % errDetails;
            return nullptr;
        }
        Node* res = interpretNode(context, srcNode);
        if ( !res )
            REP_ERROR(srcNode->location(), "Invalid node argument (%1%)") % errDetails;
        return res;
    }

    TypeRef readType(CompilationContext* context, SimpleAstNode* typeNode, const char* errDetails)
    {
        if ( !typeNode->isIdentifier() )
        {
            REP_ERROR(typeNode->location(), "Expecting identifier that denotes a type (%1%)") % errDetails;
            return nullptr;
        }

        // Check for 'void'
        if ( typeNode->stringValue() == "void" )
        {
            checkNoChildren(typeNode);
            return getVoidType(modeRtCt);
        }
        // Check for 'ref'
        else if ( typeNode->stringValue() == "ref" )
        {
            checkChildrenCount(typeNode, 1, "<type>");
            TypeRef baseType = readType(context, typeNode->children()[0], "<type>");
            if ( !baseType )
                return nullptr;
            return Feather::addRef(baseType);
        }
        // Check for 'ct'
        else if ( typeNode->stringValue() == "ct" )
        {
            checkChildrenCount(typeNode, 1, "<type>");
            TypeRef baseType = readType(context, typeNode->children()[0], "<type>");
            if ( !baseType )
                return nullptr;
            return Feather::changeTypeMode(baseType, modeCt, typeNode->location());
        }
        else
        {
            // Search for the identifier in the current symbol tab to find a class with the same name
            Node* cls = findDefinition(nkFeatherDeclClass, context, typeNode->stringValue(), typeNode->location(), "class name");
            if ( !Nest_computeType(cls) )   // TODO (simplification): Directly return the result of computeType
                return NULL;
            return getDataType(cls);
        }
    }

    bool testIdentifier(SimpleAstNode* srcNode, const char* identifier)
    {
        return srcNode->isIdentifier() && srcNode->stringValue() == identifier;
    }

    bool testReadProperty(const string& ident, SimpleAstNode* srcNode, Node* node)
    {
        string id = boost::algorithm::to_lower_copy(ident);

        if ( id == "autoct" )
        {
            checkNoChildren(srcNode);
            setEvalMode(node, modeRtCt);
            return true;
        }

        if ( id == "propstr" )
        {
            checkChildrenCount(srcNode, 2, "<name>, <value>");
            string name = readString(srcNode->children()[0], "<name>");
            string value = readString(srcNode->children()[1], "<value>");
            Nest_setPropertyString(node, name.c_str(), fromString(value));
            return true;
        }

        if ( id == "propint" )
        {
            checkChildrenCount(srcNode, 2, "<name>, <value>");
            string name = readString(srcNode->children()[0], "<name>");
            int value = readInt(srcNode->children()[1], "<value>");
            Nest_setPropertyInt(node, name.c_str(), value);
            return true;
        }

        if ( id == "propbool" )
        {
            checkChildrenCount(srcNode, 2, "<name>, <value>");
            string name = readString(srcNode->children()[0], "<name>");
            int value = readInt(srcNode->children()[1], "<value>");
            Nest_setPropertyInt(node, name.c_str(), value);
            return true;
        }

        return false;
    }



    Node* interpretNodeList(CompilationContext* context, SimpleAstNode* srcNode)
    {
        Node* res = mkNodeList(srcNode->location(), {});
        Nest_setContext(res, context);
        for ( auto child: srcNode->children() )
        {
            Node* childNode = readNode(Nest_childrenContext(res), child, "<node list child>");
            if ( !childNode ) continue;
            Nest_appendNodeToArray(&res->children, childNode);
        }
        return res;
    }

    Node* interpretNop(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkNoChildren(srcNode);
        Node* res = mkNop(srcNode->location());
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretChangeMode(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 2, "<exp>, <mode>");
        Node* exp = readNode(context, srcNode->children()[0], "<exp>");
        const string& modeStr = srcNode->children()[1]->stringValue();
        EvalMode mode = modeRtCt;
        if ( modeStr == "ct" )
            mode = modeCt;
        else if ( modeStr == "rt" )
            mode = modeRt;
        else if ( modeStr == "rtct" )
            mode = modeRtCt;
        else
            REP_ERROR(srcNode->children()[1]->location(), "Invalid evaluation mode found: '%1%'") % modeStr;
        Node* res = mkChangeMode(srcNode->location(), exp, mode);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretBackendCode(CompilationContext* context, SimpleAstNode* srcNode)
    {
        // Read all the parameters - make sure they are strings
        // Concatenate all the strings to produce the final code
        string code;
        for ( auto child: srcNode->children() )
        {
            string c = readString(child, "<code string>");
            code += c;
            code += "\n";
        }
        Node* res = mkBackendCode(srcNode->location(), fromString(code));
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretTempDestructAction(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 1, "<destruct action>");
        Node* action = readNode(context, srcNode->children()[0], "<destruct action>");
        Node* res = mkTempDestructAction(srcNode->location(), action);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretScopeDestructAction(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 1, "<destruct action>");
        Node* action = readNode(context, srcNode->children()[0], "<destruct action>");
        Node* res = mkScopeDestructAction(srcNode->location(), action);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretGlobalDestructAction(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 1, "<destruct action>");
        Node* action = readNode(context, srcNode->children()[0], "<destruct action>");
        Node* res = mkGlobalDestructAction(srcNode->location(), action);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretGlobalConstructAction(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 1, "<construct action>");
        Node* action = readNode(context, srcNode->children()[0], "<construct action>");
        Node* res = mkGlobalConstructAction(srcNode->location(), action);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretLocalSpace(CompilationContext* context, SimpleAstNode* srcNode)
    {
        NodeArray children = Nest_allocNodeArray(0);
        for ( auto child: srcNode->children() )
        {
            Node* childNode = readNode(context, child, "<local space item>");
            if ( !childNode ) continue;
            Nest_appendNodeToArray(&children, childNode);
        }
        Node* res = mkLocalSpace(srcNode->location(), all(children));
        Nest_freeNodeArray(children);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretCtValue(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 2, "<type>, <value>");
        TypeRef type = readType(context, srcNode->children()[0], "<type>");
        if ( !type )
            return nullptr;
        string val = readString(srcNode->children()[1], "<value>");
        Node* res = mkCtValue(srcNode->children()[1]->location(), type, fromString(val));
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretCtValueBin(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 2, "<type>, <bin-value>");
        TypeRef type = readType(context, srcNode->children()[0], "<type>");
        if ( !type )
            return nullptr;
        string val = readString(srcNode->children()[1], "<bin-value>");
        const Location& loc = srcNode->children()[1]->location();
        Node* res = mkCtValue(loc, type, fromString(decodeBinaryValue(loc, val)));
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretMemLoad(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 1, 5, "<address>, [<alignment>], [volatile], [unordered|monotonic|acquire|release|acquirerelease|seqconsistent], [singlethread]");
        Node* arg = readNode(context, srcNode->children()[0], "<address>");
        int alignment = 0;
        bool isVolatile = false;
        bool isSingleThread = false;
        AtomicOrdering ordering = atomicNone;
        if ( srcNode->children().size() >= 2 )
        {
            alignment = readInt(srcNode->children()[1], "<alignment>");
        }
        for ( size_t i=3; i<srcNode->children().size(); ++ i )
        {
            if ( testIdentifier(srcNode->children()[i], "volatile") )
                isVolatile = true;
            else if ( testIdentifier(srcNode->children()[i], "singlethread") )
                isSingleThread = true;
            else if ( testIdentifier(srcNode->children()[i], "unordered") )
                ordering = atomicUnordered;
            else if ( testIdentifier(srcNode->children()[i], "monotonic") )
                ordering = atomicMonotonic;
            else if ( testIdentifier(srcNode->children()[i], "acquire") )
                ordering = atomicAcquire;
            else if ( testIdentifier(srcNode->children()[i], "release") )
                ordering = atomicRelease;
            else if ( testIdentifier(srcNode->children()[i], "acquirerelease") )
                ordering = atomicAcquireRelease;
            else if ( testIdentifier(srcNode->children()[i], "seqconsistent") )
                ordering = atomicSeqConsistent;
            else
            {
                REP_ERROR(srcNode->children()[i]->location(), "Invalid argument for MemLoad (%1%)") % srcNode->children()[i]->stringValue();
            }
        }
        Node* res = mkMemLoad(srcNode->location(), arg, alignment, isVolatile, ordering, isSingleThread);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretMemStore(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 1, 6, "<value>, <address>, [<alignment>], [volatile], [unordered|monotonic|acquire|release|acquirerelease|seqconsistent], [singlethread]");
        Node* value = readNode(context, srcNode->children()[0], "<value>");
        Node* address = readNode(context, srcNode->children()[1], "<address>");
        int alignment = 0;
        bool isVolatile = false;
        bool isSingleThread = false;
        AtomicOrdering ordering = atomicNone;
        if ( srcNode->children().size() > 2 )
        {
            alignment = readInt(srcNode->children()[2], "<alignment>");
        }
        for ( size_t i=3; i<srcNode->children().size(); ++ i )
        {
            if ( testIdentifier(srcNode->children()[i], "volatile") )
                isVolatile = true;
            else if ( testIdentifier(srcNode->children()[i], "singlethread") )
                isSingleThread = true;
            else if ( testIdentifier(srcNode->children()[i], "unordered") )
                ordering = atomicUnordered;
            else if ( testIdentifier(srcNode->children()[i], "monotonic") )
                ordering = atomicMonotonic;
            else if ( testIdentifier(srcNode->children()[i], "acquire") )
                ordering = atomicAcquire;
            else if ( testIdentifier(srcNode->children()[i], "release") )
                ordering = atomicRelease;
            else if ( testIdentifier(srcNode->children()[i], "acquirerelease") )
                ordering = atomicAcquireRelease;
            else if ( testIdentifier(srcNode->children()[i], "seqconsistent") )
                ordering = atomicSeqConsistent;
            else
            {
                REP_ERROR(srcNode->children()[i]->location(), "Invalid argument for MemStore (%1%)") % srcNode->children()[i]->stringValue();
            }
        }
        Node* res = mkMemStore(srcNode->location(), value, address, alignment, isVolatile, ordering, isSingleThread);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretVarRef(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 1, "<var-name>");
        string varName = readIdentifier(srcNode->children()[0], "<var-name>");
        Node* var = findDefinition(nkFeatherDeclVar, context, varName, srcNode->children()[0]->location(), "variable");
        Node* res = mkVarRef(srcNode->location(), var);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretFieldRef(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 3, "<obj>, <class-name>, <field-name>");
        Node* obj = readNode(context, srcNode->children()[0], "<obj>");
        string className = readIdentifier(srcNode->children()[1], "<class-name>");
        string fieldName = readIdentifier(srcNode->children()[2], "<field-name>");
        Node* cls = findDefinition(nkFeatherDeclClass, context, className, srcNode->children()[1]->location(), "class");
        if ( !Nest_computeType(cls) )
            return NULL;
        Node* field = findDefinition(nkFeatherDeclVar, cls->childrenContext->currentSymTab, fieldName, srcNode->children()[2]->location(), "field", true);
        Node* res = mkFieldRef(srcNode->location(), obj, field);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretFunCall(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 1, 100, "<function-name>, [<arguments>]");
        string funName = readIdentifier(srcNode->children()[0], "<function-name>");
        Node* funDecl = findDefinition(nkFeatherDeclFunction, context, funName, srcNode->children()[0]->location(), "function");
        NodeArray args = Nest_allocNodeArray(0);
        for ( size_t i=1; i<srcNode->children().size(); ++i )
        {
            Nest_appendNodeToArray(&args, readNode(context, srcNode->children()[i], "<argument>"));
        }
        
        Node* res = mkFunCall(srcNode->location(), funDecl, all(args));
        Nest_freeNodeArray(args);
        return res;
    }

    Node* interpretReturn(CompilationContext* context, SimpleAstNode* srcNode)
    {
        Node* res;
        if ( srcNode->children().empty() )
        {
            res = mkReturn(srcNode->location(), nullptr);;
        }
        else
        {
            checkChildrenCount(srcNode, 1, "<expression>");
            Node* exp = readNode(context, srcNode->children()[0], "<expression>");
            res = mkReturn(srcNode->location(), exp);
        }
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretIf(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 2, 3, "<condition>, <then-clause>, [<else-clause>]");
        Node* cond = readNode(context, srcNode->children()[0], "<condition>");
        Node* thenClause = readNode(context, srcNode->children()[1], "<then-clause>");
        Node* elseClause = nullptr;
        if ( srcNode->children().size() >= 3 )
            elseClause = readNode(context, srcNode->children()[2], "<else-clause>");
        Node* res = mkIf(srcNode->location(), cond, thenClause, elseClause);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretStaticIf(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 2, 3, "<condition>, <then-clause>, [<else-clause>]");
        Node* cond = readNode(context, srcNode->children()[0], "<condition>");
        Node* thenClause = readNode(context, srcNode->children()[1], "<then-clause>");
        Node* elseClause = nullptr;
        if ( srcNode->children().size() >= 3 )
            elseClause = readNode(context, srcNode->children()[2], "<else-clause>");
        Node* res = mkIf(srcNode->location(), cond, thenClause, elseClause, true);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretWhile(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 2, 3, "<condition>, <body>, [<step>]");
        Node* cond = readNode(context, srcNode->children()[0], "<condition>");
        Node* body = readNode(context, srcNode->children()[1], "<body>");
        Node* step = nullptr;
        if ( srcNode->children().size() >= 3 )
            step = readNode(context, srcNode->children()[2], "<step>");
        Node* res = mkWhile(srcNode->location(), cond, body, step);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretBreak(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkNoChildren(srcNode);
        Node* res = mkBreak(srcNode->location());
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretContinue(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkNoChildren(srcNode);
        Node* res = mkContinue(srcNode->location());
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretVar(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 2, 3, "<name>, <type>, [<alignment>]");
        string name = readIdentifier(srcNode->children()[0], "<name>");
        TypeRef type = readType(context, srcNode->children()[1], "<type>");
        if ( !type )
            return nullptr;
        int alignment = 0;
        if ( srcNode->children().size() >= 3 )
        {
            alignment = readInt(srcNode->children()[2], "<alignment>");
        }
        
        Node* res = Feather::mkVar(srcNode->location(), fromString(name), mkTypeNode(srcNode->children()[1]->location(), type), alignment);
        Nest_setContext(res, context);
        return res;
    }

    Node* interpretClass(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 1, 100, "<class-name>, [<fields>]");
        string name = readIdentifier(srcNode->children()[0], "<class-name>");
        Node* cls = mkClass(srcNode->location(), fromString(name), {});
        Nest_setContext(cls, context);

        NodeArray fields = Nest_allocNodeArray(0);

        for ( size_t i=1; i<srcNode->children().size(); ++i )
        {
            string ident = readIdentifier(srcNode->children()[i], "class child");
            if ( ident == "field" )
            {
                try
                {
                    checkChildrenCountRange(srcNode->children()[i], 2, 3, "<name>, <type>, [<alignment>]");
                    Node* f = interpretVar(Nest_childrenContext(cls), srcNode->children()[i]);
                    Nest_appendNodeToArray(&fields, f);
                }
                catch(...)
                {
                }
                continue;
            }
            if ( testReadProperty(ident, srcNode->children()[i], cls) )
                continue;

            REP_ERROR(srcNode->children()[i]->location(), "Invalid identifier in class (%1%)") % ident;
        }
        Nest_appendNodesToArray(&cls->children, all(fields));
        Nest_freeNodeArray(fields);
        return cls;
    }

    Node* interpretFunction(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 3, 4, "<function-name>, params(...), <result-type>, [<body>]");
        string name = readIdentifier(srcNode->children()[0], "<function-name>");
        TypeRef resultType = readType(context, srcNode->children()[2], "<result-type>");
        if ( !resultType )
            return nullptr;

        // Create the function
        Node* fun = Feather::mkFunction(srcNode->location(), fromString(name), mkTypeNode(srcNode->children()[2]->location(), resultType), {}, nullptr);
        Nest_setContext(fun, context);

        // Read the parameters
        if ( !testIdentifier(srcNode->children()[1], "params") )
            REP_ERROR(srcNode->children()[1]->location(), "Expected params(...) as second argument to function");
        for ( auto p: srcNode->children()[1]->children() )
        {
            if ( !testIdentifier(p, "var") )
                REP_ERROR(p->location(), "Expected var(<name>, <type>, [<alignment>])");
            Node* param = interpretVar(Nest_childrenContext(fun), p);
            Function_addParameter(fun, param);
        }

        // Read the body
        if ( srcNode->children().size() >= 4 )
        {
            Node* body = readNode(Nest_childrenContext(fun), srcNode->children()[3], "<body>");
            Function_setBody(fun, body);
        }

        return fun;
    }


    Node* interpretNode(CompilationContext* context, SimpleAstNode* srcNode)
    {
        // Make sure we start with an identifier
        if ( !srcNode->isIdentifier() )
            REP_ERROR_RET(nullptr, srcNode->location(), "Expecting identifier");

        // lowercase the name
        string id = srcNode->stringValue();
        boost::algorithm::to_lower(id);

        Node* res = nullptr;
        if ( id == "nodelist" )
        {
            res = interpretNodeList(context, srcNode);
        }
        else if ( id == "nop" )
        {
            res = interpretNop(context, srcNode);
        }
        else if ( id == "changemode" )
        {
            res = interpretChangeMode(context, srcNode);
        }
        else if ( id == "backendcode" )
        {
            res = interpretBackendCode(context, srcNode);
        }
        else if ( id == "tempdestructaction" )
        {
            res = interpretTempDestructAction(context, srcNode);
        }
        else if ( id == "scopedestructaction" )
        {
            res = interpretScopeDestructAction(context, srcNode);
        }
        else if ( id == "globaldestructaction" )
        {
            res = interpretGlobalDestructAction(context, srcNode);
        }
        else if ( id == "globalconstructaction" )
        {
            res = interpretGlobalConstructAction(context, srcNode);
        }
        else if ( id == "localspace" )
        {
            res = interpretLocalSpace(context, srcNode);
        }
        else if ( id == "ctvalue" )
        {
            res = interpretCtValue(context, srcNode);
        }
        else if ( id == "ctvaluebin" )
        {
            res = interpretCtValueBin(context, srcNode);
        }
        else if ( id == "memload" )
        {
            res = interpretMemLoad(context, srcNode);
        }
        else if ( id == "memstore" )
        {
            res = interpretMemStore(context, srcNode);
        }
        else if ( id == "varref" )
        {
            res = interpretVarRef(context, srcNode);
        }
        else if ( id == "fieldref" )
        {
            res = interpretFieldRef(context, srcNode);
        }
        else if ( id == "funcall" )
        {
            res = interpretFunCall(context, srcNode);
        }
        else if ( id == "return" )
        {
            res = interpretReturn(context, srcNode);
        }
        else if ( id == "if" )
        {
            res = interpretIf(context, srcNode);
        }
        else if ( id == "staticif" )
        {
            res = interpretStaticIf(context, srcNode);
        }
        else if ( id == "while" )
        {
            res = interpretWhile(context, srcNode);
        }
        else if ( id == "break" )
        {
            res = interpretBreak(context, srcNode);
        }
        else if ( id == "continue" )
        {
            res = interpretContinue(context, srcNode);
        }
        else if ( id == "var" )
        {
            res = interpretVar(context, srcNode);
        }
        else if ( id == "localvar" )
        {
            res = interpretVar(context, srcNode);
        }
        else if ( id == "class" )
        {
            res = interpretClass(context, srcNode);
        }
        else if ( id == "function" )
        {
            res = interpretFunction(context, srcNode);
        }

        if ( !res )
            REP_ERROR(srcNode->location(), "Unknown construct found: %1%") % srcNode->stringValue();
        if ( res )
        {
            res->location = srcNode->location();
            if ( !res->context )
                Nest_setContext(res, context);
        }
        return res;
    }
}



SimpleParser::SimpleParser(SimpleLexer& lexer)
    : lexer_(lexer)
{
}

SimpleAstNode* SimpleParser::parseSourceNode()
{
    // Get the next token
    SimpleToken t = lexer_.consumeToken();

    // Check for string literals
    if ( t.tokenType() == tokenString )
    {
        return SimpleAstNode::stringConstant(t.location(), t.getValueAsString());
    }

    // Check for integer literals
    if ( t.tokenType() == tokenInteger )
    {
        return SimpleAstNode::intConstant(t.location(), t.getValueAsInt());
    }

    // Make sure the token is an identifier
    if ( t.tokenType() != tokenIdentifier )
        REP_ERROR_RET(nullptr, t.location(), "Expecting identifier or string literal");

    // Create the AST node
    SimpleAstNode* node = SimpleAstNode::identifier(t.location(), string(t.getValueAsIdentifier()));

    // Check for children - with parenthesis
    SimpleToken nextToken = lexer_.peekToken();
    if ( nextToken.tokenType() == tokenLParen )
    {
        lexer_.consumeToken();
        bool firstTime = true;

        while ( (nextToken = lexer_.peekToken()).tokenType() != tokenRParen )
        {
            // Expect a comma, if not the first time
            if ( !firstTime )
            {
                SimpleToken comma = lexer_.consumeToken();
                if ( comma.tokenType() != tokenComma )
                    REP_ERROR(comma.location(), "Expecting a comma");
            }
            firstTime = false;

            // Parse a child item
            node->addChild(parseSourceNode());
        }
        // Consume the RParen token
        lexer_.consumeToken();
    }
    else if ( nextToken.tokenType() == tokenLBrace )      // Check for children - with curly braces
    {
        lexer_.consumeToken();

        while ( (nextToken = lexer_.peekToken()).tokenType() != tokenRBrace )
        {
            // Parse a child item
            node->addChild(parseSourceNode());
        }
        // Consume the RBrace token
        lexer_.consumeToken();
    }
    return node;
}

Node* SimpleParser::parse(CompilationContext* context)
{
    return interpret(context, parseSourceNode());
}

Node* SimpleParser::interpret(CompilationContext* context, SimpleAstNode* srcNode)
{
    Node* n = interpretNode(context, srcNode);
    Nest_setContext(n, context);
    return n;
}
