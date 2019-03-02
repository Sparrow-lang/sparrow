#include "Nest/src/StdInc.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"

using Nest::NodeHandle;

namespace Nest {

vector<NodeHandle> NodeRange::toVec() const { return vector<NodeHandle>(begin(), end()); }

const char* NodeRange::toString() const {
    ostringstream ss;
    ss << *this;
    return strdup(ss.str().c_str());
}

vector<NodeHandle> NodeRangeM::toVec() const { return vector<NodeHandle>(begin(), end()); }

const char* NodeRangeM::toString() const { return NodeRange(range).toString(); }

ostream& operator<<(ostream& os, NodeRange r) {
    os << '[';
    bool first = true;
    for (auto node : r) {
        if (first)
            first = false;
        else
            os << ", ";
        if (node)
            os << node.toString();
        else
            os << "null";
    }
    os << ']';
    return os;
}

} // namespace Nest
