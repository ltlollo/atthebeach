#ifndef IOBUFFER_H
#define IOBUFFER_H

#include <strings.h>

template<typename T, size_t N>
struct Conf {
    using type = T;
    static constexpr size_t size = N;
};

struct Err {
    int err;
    size_t pos;
};

template<typename Cfg, typename Cn, typename Ds>
struct IOBuffer {
    static constexpr size_t size = Cfg::size;
    using type = typename Cfg::type;
    type storage[size];
    Cn cnstr;
    Ds destr;
    int err;
    IOBuffer(Cn c, Ds d) noexcept : cnstr{c}, destr{d} {
        bzero(storage, size*sizeof(type));
    }
    auto group_cnstr() noexcept {
        for (size_t i = 0; i < size; ++i) {
            if ((err = cnstr(storage[i]))) {
                return Err{ err, i };
            }
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

template<typename Cfg, typename Cn>
struct IOBuffer<Cfg, Cn, void> {
    static constexpr size_t size = Cfg::size;
    using type = typename Cfg::type;
    type storage[size];
    Cn cnstr;
    int err;
    IOBuffer(Cn c) noexcept : cnstr{c} {
    }
    auto group_cnstr() noexcept {
        for (size_t i = 0; i < size; ++i) {
            if ((err = cnstr(storage[i]))) {
                return Err{ err, i };
            }
        }
        return Err{ 0, size };
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
    return IOBuffer<C, F, G>{f, g};
}

template<typename C, typename F>
auto make_iobuffer(C, F f) noexcept {
    return IOBuffer<C, F, void>{f};
}

#endif // IOBUFFER_H
