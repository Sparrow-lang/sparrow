#include <StdInc.h>
#include "Literal.h"
#include "Identifier.h"
#include <NodeCommonsCpp.h>
#include <Helpers/SprTypeTraits.h>

#include <Feather/Util/StringData.h>

using namespace SprFrontend;
using namespace Nest;

Literal::Literal(const Location& loc, string litType, string data)
    : DynNode(classNodeKind(), loc)
{
    setProperty("spr.literalType", litType);
    setProperty("spr.literalData", data);
}

bool Literal::isString() const
{
    const string& litType = getCheckPropertyString("spr.literalType");
    return litType == "StringRef";
}

string Literal::asString() const
{
    return getCheckPropertyString("spr.literalData");
}

void Literal::dump(ostream& os) const
{
    const string& litType = getCheckPropertyString("spr.literalType");
    const string& data = getCheckPropertyString("spr.literalData");

    if ( litType == "StringRef" )
        os << data;
    else if ( litType == "Int" )
    {
        os << *((int*) (void*) data.c_str());
    }
    else if ( litType == "Bool" )
    {
        os << *((bool*) (void*) data.c_str());
    }
    else if ( litType == "Char" )
    {
        os << "'" << *((char*) (void*) data.c_str()) << "'";
    }
    else
        os << "literal";
}

void Literal::doSemanticCheck()
{
    using namespace Nest;

    const string& litType = getCheckPropertyString("spr.literalType");
    const string& data = getCheckPropertyString("spr.literalData");

    // Get the type of the literal by looking up the type name
    Identifier ident(data_.location, litType);
    Nest::setContext(ident.node(), data_.context);
    Nest::computeType(ident.node());
    TypeRef t = getType(ident.node());
    t = Feather::changeTypeMode(t, modeCt, data_.location);
    
    if ( litType == "StringRef" )
    {
        // Create the explanation
        Feather::StringData s = Feather::StringData::copyStdString(data);
        setExplanation(Feather::mkCtValue(data_.location, t, &s));
    }
    else
        setExplanation(Feather::mkCtValue(data_.location, t, data));
}
