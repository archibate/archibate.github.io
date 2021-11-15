#include <vector>
#include <iostream>
#include <tuple>


template <class It>
struct range {
    It m_begin;
    It m_end;

    constexpr range(It begin, It end)
        : m_begin(std::move(begin)), m_end(std::move(end))
    {}

    constexpr It begin() const { return m_begin; }
    constexpr It end() const { return m_end; }
};

template <class It>
range(It, It) -> range<It>;


template <class F>
struct pipable
{
    F m_f;

    constexpr pipable(F f)
        : m_f(std::move(f))
    {}

    template <class ...Rs>
    constexpr decltype(auto) operator()(Rs &&...rs) const {
        return m_f(range(rs.begin(), rs.end())...);
    }

    template <class R>
    friend constexpr decltype(auto) operator|(R &&r, pipable const &self) {
        return self(std::forward<decltype(r)>(r));
    }
};


template <class Func, class Base>
struct map_iterator {
    Func m_func;
    Base m_it;

    constexpr decltype(auto) operator*() const {
        return m_func(*m_it);
    }

    constexpr map_iterator &operator++() {
        ++m_it;
        return *this;
    }

    constexpr bool operator!=(map_iterator const &that) const {
        return m_it != that.m_it;
    }
};

template <class Func, class Base>
map_iterator(Func, Base) -> map_iterator<Func, Base>;

template <class F>
static constexpr auto map(F &&f) {
    return pipable([=] (auto &&r) {
        return range
            ( map_iterator{f, r.begin()}
            , map_iterator{f, r.end()}
            );
    });
}


template <class Base>
struct enumerate_iterator {
    Base m_it;
    std::size_t m_index = 0;

    constexpr decltype(auto) operator*() const {
        return std::pair<std::size_t, decltype(*m_it)>(m_index, *m_it);
    }

    constexpr enumerate_iterator &operator++() {
        ++m_it;
        ++m_index;
        return *this;
    }

    constexpr bool operator!=(enumerate_iterator const &that) const {
        return m_it != that.m_it;
    }
};

template <class Base>
enumerate_iterator(Base) -> enumerate_iterator<Base>;

static constexpr auto enumerate = pipable([] (auto &&r) {
    return range
        ( enumerate_iterator{r.begin()}
        , enumerate_iterator{r.end()}
        );
});


template <class ...Bases>
struct zip_iterator {
    std::tuple<Bases...> m_it;

    template <std::size_t ...Is>
    constexpr decltype(auto) _helper_star(std::index_sequence<Is...>) const {
        return std::tuple<decltype(*std::get<Is>(m_it))...>(*std::get<Is>(m_it)...);
    }

    constexpr decltype(auto) operator*() const {
        return _helper_star(std::make_index_sequence<sizeof...(Bases)>{});
    }

    template <std::size_t ...Is>
    constexpr void _helper_inc(std::index_sequence<Is...>) {
        (++std::get<Is>(m_it), ...);
    }

    constexpr zip_iterator &operator++() {
        _helper_inc(std::make_index_sequence<sizeof...(Bases)>{});
        return *this;
    }

    template <std::size_t ...Is>
    constexpr bool _helper_neq(zip_iterator const &that, std::index_sequence<Is...>) const {
        return ((std::get<Is>(m_it) != std::get<Is>(that.m_it)) && ...);
    }

    constexpr bool operator!=(zip_iterator const &that) const {
        return _helper_neq(that, std::make_index_sequence<sizeof...(Bases)>{});
    }
};

template <class ...Bases>
zip_iterator(std::tuple<Bases...> &&) -> zip_iterator<Bases...>;

static constexpr auto zip = pipable([] (auto &&...rs) {
    return range
        ( zip_iterator{std::tuple<decltype(rs.begin())...>{rs.begin()...}}
        , zip_iterator{std::tuple<decltype(rs.end())...>{rs.end()...}}
        );
});


int main() {
    std::vector<int> list1 = {1, 2, 3, 4};
    std::vector<int> list2 = {6, 5, 4, 3, 2, 1};
    std::vector<float> list3 = {3.14f, 2.718f, 1.414f};
    for (auto &&x: list1 | map([] (auto &&x) { return x + 1; })) {
        std::cout << x << std::endl;
    }
    for (auto &&[x, y]: enumerate(list1)) {
        y += 114514;
        std::cout << x << ' ' << y << std::endl;
    }
    for (auto &&[x, y, z]: zip(list1, list2, list3)) {
        y -= 32;
        std::cout << x << ' ' << y << ' ' << z << std::endl;
    }
    for (auto &&x: list2) {
        std::cout << x << std::endl;
    }
    return 0;
}
