#include "Nest/src/StdInc.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"

using Nest::NodeHandle;

namespace Nest {

const char* NodeRange::toString() const {
    ostringstream ss;
    ss << '[';
    bool first = true;
    for (auto node : *this) {
        if (first)
            first = false;
        else
            ss << ", ";
        if (node)
            ss << node.toString();
        else
            ss << "null";
    }
    ss << ']';
    return strdup(ss.str().c_str());
}

const char* NodeRangeM::toString() const { return NodeRange(range).toString(); }

} // namespace Nest
