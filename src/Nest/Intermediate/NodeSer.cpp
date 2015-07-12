#include <StdInc.h>
#include "NodeSer.h"
#include "Node.h"
#include "Type.h"
#include <Common/Ser/OutArchive.h>
#include <Common/Ser/InArchive.h>

using namespace Nest;
using namespace Nest::Common::Ser;

void Nest::save(const Node& obj, OutArchive& ar)
{
    // TODO (nodes): Make serialization work
//    unsigned char flags = (unsigned char) *reinterpret_cast<const unsigned int*>(&obj.data_.flags);
    ar.write("kind", obj.nodeKind);
    ar.write("kindName", nodeKindName(&obj));
//    ar.write("flags", flags);
    ar.write("location", obj.location);
    ar.writeArray("children", obj.children, [] (OutArchive& ar, Node* child) {
        ar.write("", child);
    });
    ar.writeArray("referredNodes", obj.referredNodes, [] (OutArchive& ar, Node* node) {
        ar.write("", node);
    });
    ar.writeArray("properties", obj.properties, [] (OutArchive& ar, const PropertyVal& prop) {
        ar.write("", prop, [] (OutArchive& ar, const PropertyVal& prop) {
            ar.write("name", prop.first);
            ar.write("kind", (int) prop.second.kind_);
            ar.write("passToExpl", (unsigned char) prop.second.passToExpl_);
            switch ( prop.second.kind_ )
            {
            case propInt:
                ar.write("val", prop.second.value_.intValue_);
                break;
            case propString:
                ar.write("val", *prop.second.value_.stringValue_);
                break;
            case propNode:
                ar.write("val", prop.second.value_.nodeValue_);
                break;
            case propType:
                ar.write("val", prop.second.value_.typeValue_);
                break;
            }
        });
    });
    ar.write("type", obj.type);
    ar.write("explanation", obj.explanation);
}

void Nest::load(Node& obj, InArchive& ar)
{
    // TODO
}
