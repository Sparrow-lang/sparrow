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
    if ( !type_ || type_ != other.type_ )
        return false;

    return valueData() == other.valueData();
}

bool CtValue::operator !=(const CtValue& other) const
{
    return !(*this == other);
}


void CtValue::dump(ostream& os) const
{
    if ( !type_ )
    {
        os << "ctValue";
        return;
    }
    os << "ctValue(" << type_ << ": ";
    
    const string* nativeName = type_->hasStorage ? Feather::nativeName(type_) : nullptr;
    if ( 0 == strcmp(type_->description, "Type/ct") )
    {
        TypeRef t = *this->value<TypeRef>();
        os << t;
    }
    else if ( nativeName && type_->numReferences == 0 )
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
    if ( !type_ )
        type_ = valueType();
    if ( !type_ || !type_->hasStorage || type_->mode == modeRt )
        REP_ERROR(location_, "Type specified for Ct Value cannot be used at compile-time");
    
    // Make sure data size matches the size reported by the type
    size_t valueSize = theCompiler().sizeOf(type_);
    const string& data = valueData();
    if ( valueSize != data.size() )
    {
        REP_ERROR(location_, "Read value size (%1%) differs from declared size of the value (%2%)")
            % data.size() % valueSize;
    }

    type_ = Feather::changeTypeMode(type_, modeCt, location_);
}

