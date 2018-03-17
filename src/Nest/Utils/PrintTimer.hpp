#pragma once

#include <chrono>

namespace Nest {
namespace Common {
/// Helper timer that prints the elapsed time at the console
///
/// In order for this to do something, the "enable" constructor parameter must be true; otherwise
/// this has no effect.
class PrintTimer {
    chrono::steady_clock::time_point startTime;
    const char* format;

public:
    PrintTimer(bool enable, const char* startText, const char* fmtEnd = "[%d ms]\n")
        : format(enable ? fmtEnd : nullptr) {
        if (enable) {
            printf("%s", startText);
            startTime = chrono::steady_clock::now();
        }
    }

    ~PrintTimer() {
        if (format) {
            auto durMs = chrono::duration_cast<chrono::milliseconds>(
                    chrono::steady_clock::now() - startTime);
            printf(format, durMs);
        }
    }

    PrintTimer(const PrintTimer&) = delete;
    PrintTimer(PrintTimer&&) = delete;
    const PrintTimer& operator=(const PrintTimer&) = delete;
    const PrintTimer& operator=(PrintTimer&&) = delete;
};
} // namespace Common
} // namespace Nest
