#pragma once

FWD_CLASS3(Nest,Common,Ser, OutArchive);
FWD_CLASS3(Nest,Common,Ser, InArchive);

typedef struct Nest_Node Node;

// Forward declaration
const char* Nest_toString(const Node* node);

void save(const Node& obj, Nest::Common::Ser::OutArchive& ar);
void load(Node& obj, Nest::Common::Ser::InArchive& ar);
