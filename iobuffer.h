#ifndef IOBUFFER_H
#define IOBUFFER_H

template<typename T, size_t N>
struct Conf {
    using type = T;
    static constexpr size_t size = N;
};

template<typename T>
struct Err {
    T err;
    size_t pos;
};

template<typename Cfg, typename Cn, typename Ds, typename Ret>
struct IOBuffer {
    static constexpr size_t size = Cfg::size;
    using type = typename Cfg::type;
    type storage[size]{};
    Cn cnstr;
    Ds destr;
    Ret err;
    constexpr IOBuffer(Cn c, Ds d) noexcept : cnstr{c}, destr{d} {
    }
    auto group_cnstr(const size_t pos = 0) noexcept {
        for (size_t i = pos; i < size; ++i) {
            if ((err = cnstr(storage[i]))) {
                return Err<Ret>{ err, i };
            }
        }
        return Err<Ret>{ {}, size };
    }
    auto group_destr() noexcept {
        for (size_t i = 0; i < size; ++i) {
            destr(storage[i]);
        }
    }
    auto init() noexcept {
        return group_cnstr();
    }
    auto cycle() noexcept {
        group_destr();
        return group_cnstr();
    }
};

template<typename Cfg, typename Cn, typename Ds>
struct IOBuffer<Cfg, Cn, Ds, void> {
    static constexpr size_t size = Cfg::size;
    using type = typename Cfg::type;
    type storage[size]{};
    Cn cnstr;
    Ds destr;
    constexpr IOBuffer(Cn c, Ds d) noexcept : cnstr{c}, destr{d} {
    }
    auto group_cnstr(const size_t pos = 0) noexcept {
        for (size_t i = pos; i < size; ++i) {
            cnstr(storage[i]);
        }
    }
    auto group_destr() noexcept {
        for (size_t i = 0; i < size; ++i) {
            destr(storage[i]);
        }
    }
    auto init() noexcept {
        return group_cnstr();
    }
    auto cycle() noexcept {
        group_destr();
        return group_cnstr();
    }
};

template<typename Cfg, typename Cn, typename Ret>
struct IOBuffer<Cfg, Cn, void, Ret> {
    static constexpr size_t size = Cfg::size;
    using type = typename Cfg::type;
    type storage[size];
    Cn cnstr;
    Ret err;
    constexpr IOBuffer(Cn c) noexcept : cnstr{c} {
    }
    auto group_cnstr(const size_t pos = 0) noexcept {
        for (size_t i = pos; i < size; ++i) {
            if ((err = cnstr(storage[i]))) {
                return Err<Ret>{ err, i };
            }
        }
        return Err<Ret>{ {}, size };
    }
    auto init() noexcept {
        return group_cnstr();
    }
    auto cycle() noexcept {
        return group_cnstr();
    }
};

template<typename Cfg, typename Cn>
struct IOBuffer<Cfg, Cn, void, void> {
    static constexpr size_t size = Cfg::size;
    using type = typename Cfg::type;
    type storage[size];
    Cn cnstr;
    constexpr IOBuffer(Cn c) noexcept : cnstr{c} {
    }
    auto group_cnstr(const size_t pos = 0) noexcept {
        for (size_t i = pos; i < size; ++i) {
            cnstr(storage[i]);
        }
    }
    auto init() noexcept {
        return group_cnstr();
    }
    auto cycle() noexcept {
        return group_cnstr();
    }
};

template<typename C, typename F, typename G>
auto make_iobuffer(C, F f, G g) noexcept {
    using Ret = decltype(f(declval<typename C::type&>()));
    return IOBuffer<C, F, G, Ret>{f, g};
}

template<typename C, typename F>
auto make_iobuffer(C, F f) noexcept {
    using Ret = decltype(f(declval<typename C::type&>()));
    return IOBuffer<C, F, void, Ret>{f};
}

#endif // IOBUFFER_H
