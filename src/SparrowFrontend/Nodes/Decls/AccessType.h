#pragma once

namespace SprFrontend {
/// The type of access for declarations
enum AccessType {
    unspecifiedAccess = 0, //!< Unspecified; we just take the default or the parent's access type
    publicAccess,    //!< Declaration is visible by anyone; it will be made accessible with 'using'
    protectedAccess, //!< It is not be 'transported' by 'using', but it can be access with alternate
                     //!< means (e.g., operator call) by anybody
    privateAccess,   //!< It will only be accessible for the current module
};

inline const char* accessTypeToString(AccessType accessType) {
    switch (accessType) {
    case publicAccess:
        return "public";
    case protectedAccess:
        return "protected";
    case privateAccess:
        return "private";
    case unspecifiedAccess:
    default:
        return "unspecified";
    }
}
} // namespace SprFrontend
