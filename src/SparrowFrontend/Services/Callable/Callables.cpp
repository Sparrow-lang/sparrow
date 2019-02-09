#include <StdInc.h>
#include "SparrowFrontend/Services/Callable/Callables.h"
#include "SparrowFrontend/Services/Callable/Callable.h"

namespace SprFrontend {
Callables::~Callables() {
    for (auto c : callables_)
        delete c;
}
} // namespace SprFrontend