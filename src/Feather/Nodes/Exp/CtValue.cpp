#include <StdInc.h>
#include "CtValue.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/StringData.h>

#include <boost/algorithm/string/replace.hpp>


CtValue::CtValue(const Location& loc, TypeRef type, string data)
    : DynNode(classNodeKind(), loc)
{
    setProperty("valueType", type);
    setProperty("valueData", move(data));
}


TypeRef CtValue::valueType() const
{
    return getCheckPropertyType("valueType");
}

const string& CtValue::valueData() const
{
    return getCheckPropertyString("valueData");
}

bool CtValue::operator ==(const CtValue& other) const
{
    if ( !data_.type || data_.type != other.data_.type )
        return false;

    return valueData() == other.valueData();
}

bool CtValue::operator !=(const CtValue& other) const
{
    return !(*this == other);
}


void CtValue::dump(ostream& os) const
{
    if ( !data_.type )
    {
        os << "ctValue";
        return;
    }
    os << "ctValue(" << data_.type << ": ";
    
    const string* nativeName = data_.type->hasStorage ? Feather::nativeName(data_.type) : nullptr;
    if ( 0 == strcmp(data_.type->description, "Type/ct") )
    {
        TypeRef t = *this->value<TypeRef>();
        os << t;
    }
    else if ( nativeName && data_.type->numReferences == 0 )
    {
        if ( *nativeName == "i1" || *nativeName == "u1" )
        {
            bool val = 0 != (*this->value<unsigned char>());
            os << (val ? "true" : "false");
        }
        else if ( *nativeName == "i16" )
            os << *value<short>();
        else if (  *nativeName == "u16" )
            os << *value<unsigned short>();
        else if ( *nativeName == "i32" )
            os << *value<int>();
        else if (  *nativeName == "u32" )
            os << *value<unsigned int>();
        else if ( *nativeName == "i64" )
            os << *value<long long>();
        else if (  *nativeName == "u64" )
            os << *value<unsigned long long>();
        else if (  *nativeName == "StringRef" )
        {
            StringData& s = *value<StringData>();
            os << "'" << s.toStdString() << "'";
        }
        else
            os << "'" << valueData() << "'";
    }
    else
        os << "'" << valueData() << "'";
    os << ")";
}

void CtValue::doSemanticCheck()
{
    // Check the type
    if ( !data_.type )
        data_.type = valueType();
    if ( !data_.type || !data_.type->hasStorage || data_.type->mode == modeRt )
        REP_ERROR(data_.location, "Type specified for Ct Value cannot be used at compile-time");
    
    // Make sure data size matches the size reported by the type
    size_t valueSize = theCompiler().sizeOf(data_.type);
    const string& data = valueData();
    if ( valueSize != data.size() )
    {
        REP_ERROR(data_.location, "Read value size (%1%) differs from declared size of the value (%2%)")
            % data.size() % valueSize;
    }

    data_.type = Feather::changeTypeMode(data_.type, modeCt, data_.location);
}

