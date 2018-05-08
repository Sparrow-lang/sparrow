#pragma once

template <typename T1, typename T2> struct Tuple2 {
    T1 _1;
    T2 _2;

    Tuple2(T1 p1, T2 p2)
        : _1(p1)
        , _2(p2) {}

    bool operator<(const Tuple2& other) const {
        return (_1 < other._1) || (!(other._1 < _1) && (_2 < other._2));
    }
};

template <typename T1, typename T2, typename T3> struct Tuple3 {
    T1 _1;
    T2 _2;
    T3 _3;

    Tuple3(T1 p1, T2 p2, T3 p3)
        : _1(p1)
        , _2(p2)
        , _3(p3) {}

    bool operator<(const Tuple3& other) const {
        return (_1 < other._1) || (!(other._1 < _1) && (_2 < other._2)) ||
               (!(other._1 < _1) && !(other._2 < _2) && (_3 < other._3));
    }
};

template <typename T1, typename T2, typename T3, typename T4> struct Tuple4 {
    T1 _1;
    T2 _2;
    T3 _3;
    T4 _4;

    Tuple4(T1 p1, T2 p2, T3 p3, T4 p4)
        : _1(p1)
        , _2(p2)
        , _3(p3)
        , _4(p4) {}

    bool operator<(const Tuple4& other) const {
        return (_1 < other._1) || (!(other._1 < _1) && (_2 < other._2)) ||
               (!(other._1 < _1) && !(other._2 < _2) && (_3 < other._3)) ||
               (!(other._1 < _1) && !(other._2 < _2) && !(other._3 < _3) && (_4 < other._4));
    }
};

template <typename T1, typename T2>
bool operator==(const Tuple2<T1, T2>& lhs, const Tuple2<T1, T2>& rhs) {
    return lhs._1 == rhs._1 && lhs._2 == rhs._2;
}
template <typename T1, typename T2, typename T3>
bool operator==(const Tuple3<T1, T2, T3>& lhs, const Tuple3<T1, T2, T3>& rhs) {
    return lhs._1 == rhs._1 && lhs._2 == rhs._2 && lhs._3 == rhs._3;
}
template <typename T1, typename T2, typename T3, typename T4>
bool operator==(const Tuple4<T1, T2, T3, T4>& lhs, const Tuple4<T1, T2, T3, T4>& rhs) {
    return lhs._1 == rhs._1 && lhs._2 == rhs._2 && lhs._3 == rhs._3 && lhs._4 == rhs._4;
}

namespace std {
template <typename T1, typename T2> struct hash<Tuple2<T1, T2>> {
    size_t operator()(const Tuple2<T1, T2>& t) const {
        return hash<T1>()(t._1) + 393241 * hash<T2>()(t._2);
    }
};
template <typename T1, typename T2, typename T3> struct hash<Tuple3<T1, T2, T3>> {
    size_t operator()(const Tuple3<T1, T2, T3>& t) const {
        return hash<T1>()(t._1) + 393241 * hash<T2>()(t._2) + 786433 * hash<T3>()(t._3);
    }
};
template <typename T1, typename T2, typename T3, typename T4> struct hash<Tuple4<T1, T2, T3, T4>> {
    size_t operator()(const Tuple4<T1, T2, T3, T4>& t) const {
        return hash<T1>()(t._1) + 393241 * hash<T2>()(t._2) + 786433 * hash<T3>()(t._3) +
               1572869 * hash<T4>()(t._4);
    }
};
template <typename T1, typename T2, typename T3, typename T4> struct hash<tuple<T1, T2, T3, T4>> {
    size_t operator()(const tuple<T1, T2, T3, T4>& t) const {
        size_t res = hash<T1>()(std::get<0>(t));
        res += 393241 * hash<T2>()(std::get<1>(t));
        res += 786433 * hash<T3>()(std::get<2>(t));
        res += 1572869 * hash<T4>()(std::get<3>(t));
        return res;
    }
};
} // namespace std
