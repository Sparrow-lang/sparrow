#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundefined-inline"
#endif

#include "catch.hpp"
#include "internal/catch_version.h"
#include "internal/catch_text.h"
#include "reporters/catch_reporter_console.hpp"

// HACK to get around problem in catch: Repeat the function from catch_version.hpp
// catch_reporter_console.hpp needs it, but it can't find it;
// and including catch_version.hpp would produce duplicate symbols on link
inline Catch::Version Catch::libraryVersion()
{
    static Version version( 1, 9, 7, "", 0 );
    return version;
}

void printSpaces(int indent) {
    for (int i = 0; i < indent; i++)
        printf("    ");
}

struct DetailConsoleReporter : Catch::ConsoleReporter {
    DetailConsoleReporter(Catch::ReporterConfig const& _config)
        : ConsoleReporter(_config) {}

    bool assertionEnded(const Catch::AssertionStats& assStats) override {
        if (needsNewline)
            printf("\n");
        needsNewline = false;
        return ConsoleReporter::assertionEnded(assStats);
    }
    void sectionStarting(const Catch::SectionInfo& secInfo) override {
        // call the base
        ConsoleReporter::sectionStarting(secInfo);
        // ensure that we have the same number of sections in our vector
        int indent = m_sectionStack.size()-1;
        if (lastSeenSectionsStack.size() <= indent)
            lastSeenSectionsStack.resize(indent+1);
        // are we starting a section that is already on the stack?
        bool reusingSection = lastSeenSectionsStack[indent] == secInfo.name;
        lastSeenSectionsStack[indent] = secInfo.name;
        // do the printing
        if (!reusingSection) {
            if (needsNewline)
                printf("\n");
            int indent = m_sectionStack.size()-1;
            printSpaces(indent);
            printf("* %s", secInfo.name.c_str());
            needsNewline = true;
        }
    }
    void sectionEnded(const Catch::SectionStats& secStats) override {
        ConsoleReporter::sectionEnded(secStats);
        if (needsNewline) {
            if (secStats.assertions.allPassed()) {
                printf(" -> ");
                {
                    Catch::Colour colourGuard(Catch::Colour::Success);
                    printf("ok");
                }
            }
            printf("\n");
            needsNewline = false;
        }
    }

private:
    std::vector<std::string> lastSeenSectionsStack;
    bool needsNewline{false};
};

INTERNAL_CATCH_REGISTER_REPORTER("details", DetailConsoleReporter)
