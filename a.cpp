#include <array>
#include <vector>
#include <iostream>

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

// define the deduction rule:
template <class It>
range(It begin, It end) -> range<It>;


template <class F>
struct pipable
{
    F m_f;

    constexpr pipable(F f)
        : m_f(std::move(f))
    {}

    // function style: map(func)(range)
    constexpr decltype(auto) operator()(auto &&...rs) const {
        return m_f(range(rs.begin(), rs.end())...);
    }

    // pipe style: range | map(func)
    friend constexpr decltype(auto) operator|(auto &&r, pipable const &self) {
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
        m_it++;
        return *this;
    }

    constexpr bool operator!=(map_iterator const &that) const {
        return m_it != that.m_it;
    }
};

template <class Func, class Base>
map_iterator(Func, Base) -> map_iterator<Func, Base>;

static constexpr auto map(auto &&f) {
    return pipable([=] (auto &&r) {
        return range
            ( map_iterator{f, r.begin()}
            , map_iterator{f, r.end()}
            );
    });
}


int main() {
    std::vector<int> list = {1, 2, 3, 4};
    for (auto &&x: list | map([] (auto &&x) { return x + 1; })) {
        std::cout << x << std::endl;
    }
    return 0;
}
