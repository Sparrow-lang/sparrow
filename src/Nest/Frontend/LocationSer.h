#pragma once

#include "Location.h"

FWD_CLASS3(Nest,Common,Ser, OutArchive)
FWD_CLASS3(Nest,Common,Ser, InArchive)

ostream& operator << (ostream& os, const Location* loc);
ostream& operator << (ostream& os, const Location& loc);

// Serialization
void save(const Location& obj, Nest::Common::Ser::OutArchive& ar);
void load(Location& obj, Nest::Common::Ser::InArchive& ar);

