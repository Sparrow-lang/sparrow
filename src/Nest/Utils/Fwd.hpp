#pragma once

#define FWD_CLASS0(className) class className;
#define FWD_CLASS1(ns1, className)                                                                 \
    namespace ns1 {                                                                                \
    class className;                                                                               \
    }
#define FWD_CLASS2(ns1, ns2, className)                                                            \
    namespace ns1 {                                                                                \
    namespace ns2 {                                                                                \
    class className;                                                                               \
    }                                                                                              \
    }
#define FWD_CLASS3(ns1, ns2, ns3, className)                                                       \
    namespace ns1 {                                                                                \
    namespace ns2 {                                                                                \
    namespace ns3 {                                                                                \
    class className;                                                                               \
    }                                                                                              \
    }                                                                                              \
    }
#define FWD_CLASS4(ns1, ns2, ns3, ns4, className)                                                  \
    namespace ns1 {                                                                                \
    namespace ns2 {                                                                                \
    namespace ns3 {                                                                                \
    namespace ns4 {                                                                                \
    class className;                                                                               \
    }                                                                                              \
    }                                                                                              \
    }                                                                                              \
    }
#define FWD_CLASS5(ns1, ns2, ns3, ns4, ns5, className)                                             \
    namespace ns1 {                                                                                \
    namespace ns2 {                                                                                \
    namespace ns3 {                                                                                \
    namespace ns4 {                                                                                \
    namespace ns5 {                                                                                \
    class className;                                                                               \
    }                                                                                              \
    }                                                                                              \
    }                                                                                              \
    }                                                                                              \
    }

#define FWD_STRUCT0(structName) struct structName;
#define FWD_STRUCT1(ns1, structName)                                                               \
    namespace ns1 {                                                                                \
    struct structName;                                                                             \
    }
#define FWD_STRUCT2(ns1, ns2, structName)                                                          \
    namespace ns1 {                                                                                \
    namespace ns2 {                                                                                \
    struct structName;                                                                             \
    }                                                                                              \
    }
#define FWD_STRUCT3(ns1, ns2, ns3, structName)                                                     \
    namespace ns1 {                                                                                \
    namespace ns2 {                                                                                \
    namespace ns3 {                                                                                \
    struct structName;                                                                             \
    }                                                                                              \
    }                                                                                              \
    }
#define FWD_STRUCT4(ns1, ns2, ns3, ns4, structName)                                                \
    namespace ns1 {                                                                                \
    namespace ns2 {                                                                                \
    namespace ns3 {                                                                                \
    namespace ns4 {                                                                                \
    struct structName;                                                                             \
    }                                                                                              \
    }                                                                                              \
    }                                                                                              \
    }
#define FWD_STRUCT5(ns1, ns2, ns3, ns4, ns5, structName)                                           \
    namespace ns1 {                                                                                \
    namespace ns2 {                                                                                \
    namespace ns3 {                                                                                \
    namespace ns4 {                                                                                \
    namespace ns5 {                                                                                \
    struct structName;                                                                             \
    }                                                                                              \
    }                                                                                              \
    }                                                                                              \
    }                                                                                              \
    }
