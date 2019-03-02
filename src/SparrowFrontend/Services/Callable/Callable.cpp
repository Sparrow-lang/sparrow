#include <StdInc.h>
#include "SparrowFrontend/Services/Callable/Callable.h"
#include "SparrowFrontend/Services/IConvertService.h"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace SprFrontend {

int moreSpecialized(
        CompilationContext* context, const Callable& f1, const Callable& f2, bool noCustomCvt) {
    // Check parameter count
    size_t paramsCount = f1.numParams();
    if (paramsCount != f2.numParams())
        return 0;

    bool firstIsMoreSpecialized = false;
    bool secondIsMoreSpecialized = false;
    for (size_t i = 0; i < paramsCount; ++i) {
        Type t1 = f1.paramType(i);
        Type t2 = f2.paramType(i);
        // Ignore any params that are null
        // TODO (overloading): Fix this - we reach in this state for dependent params
        if (!t1 || !t2)
            continue;

        // Ignore parameters of same type
        if (t1 == t2)
            continue;

        ConversionFlags flags = noCustomCvt ? flagDontCallConversionCtor : flagsDefault;
        ConversionResult c1 = g_ConvertService->checkConversion(context, t1, t2, flags);
        if (c1) {
            firstIsMoreSpecialized = true;
            if (secondIsMoreSpecialized)
                return 0;
        }
        ConversionResult c2 = g_ConvertService->checkConversion(context, t2, t1, flags);
        if (c2) {
            secondIsMoreSpecialized = true;
            if (firstIsMoreSpecialized)
                return 0;
        }
    }
    if (firstIsMoreSpecialized && !secondIsMoreSpecialized)
        return -1;
    else if (!firstIsMoreSpecialized && secondIsMoreSpecialized)
        return 1;
    else
        return 0;
}

} // namespace SprFrontend
