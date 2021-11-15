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
        m_it++;
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
        m_it++;
        m_index++;
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


int main() {
    std::vector<int> list = {1, 2, 3, 4};
    for (auto &&x: list | map([] (auto &&x) { return x + 1; })) {
        std::cout << x << std::endl;
    }
    for (auto &&[x, y]: enumerate(list)) {
        std::cout << x << ' ' << y << std::endl;
    }
    return 0;
}
