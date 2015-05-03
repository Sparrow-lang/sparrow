#pragma once


namespace SprFrontend
{
    /**
     * \brief   The type of access for definitions
     *
     * When no access type is given for a declaration, the public access is assumed.
     */
    enum AccessType
    {
        publicAccess,
        privateAccess,
        protectedAccess,
    };

    inline string accessTypeToString(AccessType accessType)
    {
        switch (accessType)
        {
        case privateAccess:
            return "private";
        case protectedAccess:
            return "protected";
        default:
            return "public";
        }
    }
}
