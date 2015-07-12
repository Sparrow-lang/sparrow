#pragma once

FWD_STRUCT1(Nest, Node);
FWD_CLASS3(Nest,Common,Ser, OutArchive)
FWD_CLASS3(Nest,Common,Ser, InArchive)

namespace Nest
{
    // Forward declaration
    const char* toString(const Node* node);

    template <typename T>
    basic_ostream<T>& operator << (basic_ostream<T>& os, const Node* n)
    {
        if ( n )
            os << toString(n);
        return os;
    }

    void save(const Node& obj, Common::Ser::OutArchive& ar);
    void load(Node& obj, Common::Ser::InArchive& ar);
}
