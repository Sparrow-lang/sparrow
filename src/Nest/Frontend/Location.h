#pragma once

FWD_CLASS1(Nest, SourceCode)
FWD_CLASS3(Nest,Common,Ser, OutArchive)
FWD_CLASS3(Nest,Common,Ser, InArchive)

namespace Nest
{
    /// Holds a location in a source code file. The location has a start and an end position in the same source code
    class Location
    {
    public:
        Location();
        Location(const SourceCode& sourceCode);
        Location(const SourceCode& sourceCode, size_t lineNo, size_t colNo);
        Location(const SourceCode& sourceCode, size_t startLineNo, size_t startColNo, size_t endLineNo, size_t endColNo);

        /// Returns true if this not indicates a valid location
        bool empty() const;

        /// Getter for the filename string
        const SourceCode* sourceCode() const;

        /// Getter for the start line number
        size_t startLineNo() const;

        /// Getter for the start column number
        size_t startColNo() const;

        /// Getter for the start line number
        size_t endLineNo() const;

        /// Getter for the start column number
        size_t endColNo() const;

        /// Gets a short string representation of the location
        string toString() const;

        /// Gets the code corresponding to this location
        string getCorrespondingCode() const;

    // Location manipulation functions
    public:
        /// Make the start position to be the same as the end position
        void step();

        /// Add the given number of columns to the end position; start position remains unchanged
        void addColumns(size_t count = 1);

        /// Add the given number of lines to the end position; start position remains unchanged
        void addLines(size_t count = 1);

        /// Set the start position of this location to the start position of the given location; the end position remains unchanged
        void copyStart(const Location& rhs);

        /// Set the end position of this location to the end position of the given location; the start position remains unchanged
        void copyEnd(const Location& rhs);

        /// Set this location with both start and end to equal the start of the given location
        void setAsStartOf(const Location& rhs);

        /// Set this location with both start and end to equal the end of the given location
        void setAsEndOf(const Location& rhs);

    // Serialization
    public:
        void save(Common::Ser::OutArchive& ar) const;
        void load(Common::Ser::InArchive& ar);

    private:
        // The corresponding source code
        const SourceCode* sourceCode_;
        
        unsigned int startLineNo_;        /// The start line number
        unsigned int startColNo_;         /// The start column number
        unsigned int endLineNo_;          /// The ending line number
        unsigned int endColNo_;           /// The ending column number
    };

    ostream& operator << (ostream& os, const Location& loc);

    bool operator <(const Location& loc1, const Location& loc2);

    bool operator ==(const Location& l1, const Location& l2);
    bool operator !=(const Location& l1, const Location& l2);
}
