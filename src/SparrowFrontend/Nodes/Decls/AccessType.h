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

    inline string accessTypeToString(AccessType accessType)
    {
        return accessType == privateAccess ? "private" : "public";
    }
}
