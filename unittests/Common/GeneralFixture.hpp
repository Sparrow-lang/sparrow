#pragma once

#include "Nest/Api/Node.h"

//! General fixture to be used for unit testing.
//!
//! This ensures proper initialization and shutdown of Sparrow code. It ensures that we can restart
//! the compiler state after a test is run.
//!
//! It also provide some generic utilities for testing.
struct GeneralFixture {
    GeneralFixture();
    ~GeneralFixture();

    static GeneralFixture& instance();

    Location createLocation();

    Node* createDatatypeNode(StringRef name, CompilationContext* ctx = nullptr);
    Node* createNativeDatatypeNode(StringRef name, CompilationContext* ctx = nullptr);

    CompilationContext* globalContext_{nullptr};

private:
    //! The last instance of this class
    static GeneralFixture* lastInstance_;
    //! The source code we are using for all generated code
    SourceCode* sourceCode_{nullptr};
    //! The last line number we've used for locations
    int lastLineNum_{0};
};
