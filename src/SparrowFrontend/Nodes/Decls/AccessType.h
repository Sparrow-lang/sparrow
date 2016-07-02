#pragma once


namespace SprFrontend
{
    /**
     * \brief   The type of access for definitions
     *
     * When no access type is given for a declaration, the private access is assumed.
     */
    enum AccessType
    {
        publicAccess,
        privateAccess,

        unspecifiedAccess
    };

    inline const char* accessTypeToString(AccessType accessType)
    {
        switch(accessType)
        {
        case publicAccess:
            return "public";
        case privateAccess:
            return "private";
        case unspecifiedAccess:
        default:
            return "unspecified";
        }
    }
}
