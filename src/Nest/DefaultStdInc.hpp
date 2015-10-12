#pragma once


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <ctype.h>
#include <cassert>
#include <stdint.h>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/any.hpp>
#include <boost/unordered_map.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include "Common/Fwd.hpp"
#include "Common/Assert.h"

// we are pretty fond of STL
using namespace std;

// Instantiations
#ifdef _MSC_VER
template basic_string<char>;
template vector<string>;
template basic_stringstream<char>;
template basic_ostringstream<char>;
#ifndef INSIDE_NEST
template boost::basic_format<char>;     // For some strange reason, this doesn't work inside Nest
#endif
#endif

// Disable warnings in MSVC:
// "C4355: 'this' : used in base member initializer list"
// "C4996: 'stricmp': The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name: _stricmp. See online help for details"
// "C4510: 'MyClass' : default constructor could not be generated"
// "C4512: 'MyClass' : assignment operator could not be generated"
// "C4610: class 'MyClass' can never be instantiated - user defined constructor required"
// "C4480: nonstandard extension used: specifying underlying type for enum 'MyEnum'"
// "C4127: conditional expression is constant"
#ifdef _MSC_VER
#pragma warning (disable:4355)
#pragma warning (disable:4996)
#pragma warning (disable:4510)
#pragma warning (disable:4512)
#pragma warning (disable:4610)
#pragma warning (disable:4480)
#pragma warning (disable:4127)
#endif

