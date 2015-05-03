#pragma once

#include "SimpleTokenType.h"

#include <Nest/Frontend/Location.h>

#include <boost/algorithm/string/replace.hpp>

namespace Feather
{
    using Nest::Location;

    /// Structure for representing a token read from the FSimple input source code
    class SimpleToken
    {
    public:
        SimpleToken(Location location, SimpleTokenType tokenType, const char* valueBegin = nullptr, const char* valueEnd = nullptr)
            : location_(location), tokenType_(tokenType), valueBegin_(valueBegin), valueEnd_(valueEnd)
        {}

        /// Gets the location of this token
        const Location& location() const { return location_; }

        /// Gets the token type
        SimpleTokenType tokenType() const { return tokenType_; }

        /// Gets the value as an identifier
        string getValueAsIdentifier() const
        {
            return string(valueBegin_, valueEnd_);
        }

        /// Gets the value as a string constant
        string getValueAsString() const
        {
            string res(valueBegin_, valueEnd_);
            boost::algorithm::replace_all(res, "\\\"", "\"");   // Replace backslash quote with a single quote
            boost::algorithm::replace_all(res, "\\\\", "\\");   // Replace double backslash with a single backslash
            return res;
        }

        /// Gets the value as an integer
        int getValueAsInt() const
        {
            return boost::lexical_cast<int>(string(valueBegin_, valueEnd_));
        }

    private:
        Location location_; ///< The location of this token
        SimpleTokenType tokenType_;         ///< The type of the token
        const char* valueBegin_;            ///< Pointer to the character in the source code where this token starts
        const char* valueEnd_;              ///< Pointer to the last+1 character of the token

    };
}
