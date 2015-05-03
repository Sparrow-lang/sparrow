#include <StdInc.h>
#include "SimpleParser.h"
#include "SimpleLexer.h"
#include "SimpleAstNode.h"

#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Nodes/NodeList.h>
#include <Feather/Nodes/ChangeMode.h>
#include <Feather/Nodes/Exp/AtomicOrdering.h>
#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Nodes/Decls/Var.h>
#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Type/Void.h>
#include <Feather/Type/DataType.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Decl.h>

#include <Nest/Common/Diagnostic.h>
#include <Nest/Intermediate/SymTab.h>
#include <Nest/Intermediate/CompilationContext.h>

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
            REP_ERROR(loc, "Invalid binary value");
        
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
    template <typename T>
    T* findDefinition(SymTab* symTab, const string& name, const Location& loc, const char* desc, bool onlyCurrent = false)
    {
        ASSERT(symTab);
        const auto& entries = onlyCurrent ? symTab->lookupCurrent(name) : symTab->lookup(name);
        if ( entries.empty() )
        {
            REP_ERROR(loc, "Cannot find %1%") % name;
            return nullptr;
        }
        if ( entries.size() > 1 )
        {
            try
            {
                REP_ERROR(loc, "Multiple definitions found for name %1%") % name;
            }
            catch(...)
            {
                for ( Node* entry: entries )
                {
                    if ( entry )
                        REP_INFO(entry->location(), "See possible alternative");
                }
                throw;
            }
            return nullptr;
        }
        T* def = dynamic_cast<T*>(entries.front());
        if ( !def  )
        {
            REP_ERROR(loc, "Identifier %1% doesn't denote a %2%") % name % desc;
            return nullptr;
        }
        return def;
    }
    template <typename T>
    T* findDefinition(CompilationContext* context, const string& name, const Location& loc, const char* desc)
    {
        return findDefinition<T>(context->currentSymTab(), name, loc, desc);
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

    Type* readType(CompilationContext* context, SimpleAstNode* typeNode, const char* errDetails)
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
            return Feather::Void::get(modeRtCt);
        }
        // Check for 'ref'
        else if ( typeNode->stringValue() == "ref" )
        {
            checkChildrenCount(typeNode, 1, "<type>");
            Type* baseType = readType(context, typeNode->children()[0], "<type>");
            return Feather::addRef(baseType);
        }
        // Check for 'ct'
        else if ( typeNode->stringValue() == "ct" )
        {
            checkChildrenCount(typeNode, 1, "<type>");
            Type* baseType = readType(context, typeNode->children()[0], "<type>");
            return Feather::changeTypeMode(baseType, modeCt, typeNode->location());
        }
        else
        {
            // Search for the identifier in the current symbol tab to find a class with the same name
            Class* cls = findDefinition<Class>(context, typeNode->stringValue(), typeNode->location(), "class name");
            cls->computeType();
            return Feather::DataType::get(cls);
        }
    }

    bool testIdentifier(SimpleAstNode* srcNode, const char* identifier)
    {
        return srcNode->isIdentifier() && srcNode->stringValue() == identifier;
    }

    bool testReadProperty(const string& ident, SimpleAstNode* srcNode, Class* node)
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
            node->setProperty(name.c_str(), value);
            return true;
        }

        if ( id == "propint" )
        {
            checkChildrenCount(srcNode, 2, "<name>, <value>");
            string name = readString(srcNode->children()[0], "<name>");
            int value = readInt(srcNode->children()[1], "<value>");
            node->setProperty(name.c_str(), value);
            return true;
        }

        if ( id == "propbool" )
        {
            checkChildrenCount(srcNode, 2, "<name>, <value>");
            string name = readString(srcNode->children()[0], "<name>");
            int value = readInt(srcNode->children()[1], "<value>");
            node->setProperty(name.c_str(), value);
            return true;
        }

        return false;
    }



    Node* interpretNodeList(CompilationContext* context, SimpleAstNode* srcNode)
    {
        NodeList* res = (NodeList*) mkNodeList(srcNode->location(), {});
        res->setContext(context);
        for ( auto child: srcNode->children() )
        {
            Node* childNode = readNode(res->childrenContext(), child, "<node list child>");
            if ( !childNode ) continue;
            res->addChild(childNode);
        }
        return res;
    }

    Node* interpretNop(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkNoChildren(srcNode);
        Node* res = mkNop(srcNode->location());
        res->setContext(context);
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
        Node* res = new ChangeMode(srcNode->location(), mode, exp);
        res->setContext(context);
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
        Node* res = mkBackendCode(srcNode->location(), code);
        res->setContext(context);
        return res;
    }

    Node* interpretTempDestructAction(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 1, "<destruct action>");
        Node* action = readNode(context, srcNode->children()[0], "<destruct action>");
        Node* res = mkTempDestructAction(srcNode->location(), action);
        res->setContext(context);
        return res;
    }

    Node* interpretScopeDestructAction(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 1, "<destruct action>");
        Node* action = readNode(context, srcNode->children()[0], "<destruct action>");
        Node* res = mkScopeDestructAction(srcNode->location(), action);
        res->setContext(context);
        return res;
    }

    Node* interpretGlobalDestructAction(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 1, "<destruct action>");
        Node* action = readNode(context, srcNode->children()[0], "<destruct action>");
        Node* res = mkGlobalDestructAction(srcNode->location(), action);
        res->setContext(context);
        return res;
    }

    Node* interpretGlobalConstructAction(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 1, "<construct action>");
        Node* action = readNode(context, srcNode->children()[0], "<construct action>");
        Node* res = mkGlobalConstructAction(srcNode->location(), action);
        res->setContext(context);
        return res;
    }

    Node* interpretLocalSpace(CompilationContext* context, SimpleAstNode* srcNode)
    {
        NodeVector children;
        for ( auto child: srcNode->children() )
        {
            Node* childNode = readNode(context, child, "<local space item>");
            if ( !childNode ) continue;
            children.push_back(childNode);
        }
        Node* res = mkLocalSpace(srcNode->location(), children);
        res->setContext(context);
        return res;
    }

    Node* interpretCtValue(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 2, "<type>, <value>");
        Type* type = readType(context, srcNode->children()[0], "<type>");
        string val = readString(srcNode->children()[1], "<value>");
        Node* res = mkCtValue(srcNode->children()[1]->location(), type, val);
        res->setContext(context);
        return res;
    }

    Node* interpretCtValueBin(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 2, "<type>, <bin-value>");
        Type* type = readType(context, srcNode->children()[0], "<type>");
        string val = readString(srcNode->children()[1], "<bin-value>");
        const Location& loc = srcNode->children()[1]->location();
        Node* res = mkCtValue(loc, type, decodeBinaryValue(loc, val));
        res->setContext(context);
        return res;
    }

    Node* interpretStackAlloc(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 1, 3, "<type>, [num-elements], [alignment]");
        Type* type = readType(context, srcNode->children()[0], "<type>");
        int numElements = 1;
        int alignment = 0;
        if ( srcNode->children().size() >= 2 )
        {
            numElements = readInt(srcNode->children()[1], "<num-elements>");
        }
        if ( srcNode->children().size() >= 3 )
        {
            alignment = readInt(srcNode->children()[2], "<alignment>");
        }
        Node* res = mkStackAlloc(srcNode->location(), mkTypeNode(srcNode->children()[0]->location(), type), numElements, alignment);
        res->setContext(context);
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
        res->setContext(context);
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
        res->setContext(context);
        return res;
    }

    Node* interpretVarRef(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 1, "<var-name>");
        string varName = readIdentifier(srcNode->children()[0], "<var-name>");
        Node* var = findDefinition<Var>(context, varName, srcNode->children()[0]->location(), "variable");
        Node* res = mkVarRef(srcNode->location(), var);
        res->setContext(context);
        return res;
    }

    Node* interpretFieldRef(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCount(srcNode, 3, "<obj>, <class-name>, <field-name>");
        Node* obj = readNode(context, srcNode->children()[0], "<obj>");
        string className = readIdentifier(srcNode->children()[1], "<class-name>");
        string fieldName = readIdentifier(srcNode->children()[2], "<field-name>");
        Class* cls = findDefinition<Class>(context, className, srcNode->children()[1]->location(), "class");
        cls->computeType();
        Node* field = findDefinition<Var>(cls->childrenContext()->currentSymTab(), fieldName, srcNode->children()[2]->location(), "field", true);
        Node* res = mkFieldRef(srcNode->location(), obj, field);
        res->setContext(context);
        return res;
    }

    Node* interpretFunCall(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 1, 100, "<function-name>, [<arguments>]");
        string funName = readIdentifier(srcNode->children()[0], "<function-name>");
        Function* funDecl = findDefinition<Function>(context, funName, srcNode->children()[0]->location(), "function");
        NodeVector args;
        for ( size_t i=1; i<srcNode->children().size(); ++i )
        {
            args.push_back(readNode(context, srcNode->children()[i], "<argument>"));
        }
        
        return mkFunCall(srcNode->location(), funDecl, args);
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
        res->setContext(context);
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
        res->setContext(context);
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
        res->setContext(context);
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
        res->setContext(context);
        return res;
    }

    Node* interpretBreak(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkNoChildren(srcNode);
        Node* res = mkBreak(srcNode->location());
        res->setContext(context);
        return res;
    }

    Node* interpretContinue(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkNoChildren(srcNode);
        Node* res = mkContinue(srcNode->location());
        res->setContext(context);
        return res;
    }

    Node* interpretVar(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 2, 3, "<name>, <type>, [<alignment>]");
        string name = readIdentifier(srcNode->children()[0], "<name>");
        Type* type = readType(context, srcNode->children()[1], "<type>");
        int alignment = 0;
        if ( srcNode->children().size() >= 3 )
        {
            alignment = readInt(srcNode->children()[2], "<alignment>");
        }
        
        Node* res = Feather::mkVar(srcNode->location(), name, mkTypeNode(srcNode->children()[1]->location(), type), alignment);
        res->setContext(context);
        return res;
    }

    Node* interpretClass(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 1, 100, "<class-name>, [<fields>]");
        string name = readIdentifier(srcNode->children()[0], "<class-name>");
        Class* cls = (Class*) mkClass(srcNode->location(), name, {});
        cls->setContext(context);

        NodeVector fields;

        for ( size_t i=1; i<srcNode->children().size(); ++i )
        {
            string ident = readIdentifier(srcNode->children()[i], "class child");
            if ( ident == "field" )
            {
                try
                {
                    checkChildrenCountRange(srcNode->children()[i], 2, 3, "<name>, <type>, [<alignment>]");
                    Node* f = interpretVar(cls->childrenContext(), srcNode->children()[i]);
                    fields.push_back(f);
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
        cls->addFields(fields);
        return cls;
    }

    Node* interpretFunction(CompilationContext* context, SimpleAstNode* srcNode)
    {
        checkChildrenCountRange(srcNode, 3, 4, "<function-name>, params(...), <result-type>, [<body>]");
        string name = readIdentifier(srcNode->children()[0], "<function-name>");
        Type* resultType = readType(context, srcNode->children()[2], "<result-type>");

        // Create the function
        Function* fun = (Function*) Feather::mkFunction(srcNode->location(), name, mkTypeNode(srcNode->children()[2]->location(), resultType), {}, nullptr);
        fun->setContext(context);

        // Read the parameters
        if ( !testIdentifier(srcNode->children()[1], "params") )
            REP_ERROR(srcNode->children()[1]->location(), "Expected params(...) as second argument to function");
        NodeVector params;
        for ( auto p: srcNode->children()[1]->children() )
        {
            if ( !testIdentifier(p, "var") )
                REP_ERROR(p->location(), "Expected var(<name>, <type>, [<alignment>])");
            Node* param = interpretVar(fun->childrenContext(), p);
            fun->addParameter(param);
        }

        // Read the body
        if ( srcNode->children().size() >= 4 )
        {
            Node* body = readNode(fun->childrenContext(), srcNode->children()[3], "<body>");
            fun->setBody(body);
        }

        return fun;
    }


    Node* interpretNode(CompilationContext* context, SimpleAstNode* srcNode)
    {
        // Make sure we start with an identifier
        if ( !srcNode->isIdentifier() )
            REP_ERROR(srcNode->location(), "Expecting identifier");

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
        else if ( id == "stackalloc" )
        {
            res = interpretStackAlloc(context, srcNode);
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
            res->setLocation(srcNode->location());
            if ( !res->context() )
                res->setContext(context);
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
        try
        {
            return SimpleAstNode::intConstant(t.location(), t.getValueAsInt());
        }
        catch (...)
        {
            REP_ERROR(t.location(), "Invalid integer constant");
            return SimpleAstNode::intConstant(t.location(), 0);
        }
    }

    // Make sure the token is an identifier
    if ( t.tokenType() != tokenIdentifier )
        REP_ERROR(t.location(), "Expecting identifier or string literal");

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
                    REP_ERROR_NOTHROW(comma.location(), "Expecting a comma");
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
    n->setContext(context);
    return n;
}
