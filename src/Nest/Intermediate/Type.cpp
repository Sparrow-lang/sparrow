#include <StdInc.h>
#include "Type.h"
#include <Common/Ser/OutArchive.h>
#include <Common/Ser/InArchive.h>

using namespace Nest;
using namespace Nest::Common::Ser;


void Type::save(OutArchive& ar) const
{
    ar.write("typeId", typeId_);
    ar.write("mode", (char) mode_);
    ar.write("flags", flags_);
    ar.writeArray("subTypes", subTypes_, [] (OutArchive& ar, Type* t) {
        ar.write("", t);
    });
    // TODO: additional data
    ar.write("additionalData", (unsigned long long) (void*) additionalData_);

#ifdef USE_TYPE_DESCRIPTIONS
    ar.write("description", description_);
#endif
}

void Type::load(InArchive& ar)
{
    // TODO
}