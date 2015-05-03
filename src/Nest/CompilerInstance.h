#pragma once

FWD_CLASS1(Nest, Compiler)

namespace Nest
{
    /// Holds the main instance of the compiler
    /// This is a singleton and a holder for the compiler class
    class CompilerInstance
    {
    public:
        /// Getter for the only instance of this object
        static CompilerInstance& instance();

        /// Getter for the compiler object held by this class
        Compiler& compiler() const;

        /// Calls to initialize the compiler object
        /// This method should be called before any calls to the compiler() method
        void init();

        /// Calls to destroy the compiler object
        /// No more calls to the compiler() method should be made after calling this method
        void destroy();

        /// Called to reinitialize the compiler object
        /// Similar to calling destroy() and then init()
        void reset();

    private:
        CompilerInstance();

    private:
        /// The compiler instance held by this class
        Compiler* compiler_;
    };
}
