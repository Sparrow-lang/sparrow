#pragma once

FWD_STRUCT1(Nest, Node);
FWD_CLASS3(Nest,Common,Ser, OutArchive)
FWD_CLASS3(Nest,Common,Ser, InArchive)

namespace Nest
{
    // Forward declaration
    const char* toString(const Node* node);

    void save(const Node& obj, Common::Ser::OutArchive& ar);
    void load(Node& obj, Common::Ser::InArchive& ar);
}
