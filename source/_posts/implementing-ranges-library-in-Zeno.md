---
title: implementing ranges library in Zeno
date: 2021-11-15 11:05:58
tags:
---

<del>关于为什么 C++20 引入了 ranges 库却没有 zip 还要用户自己实现这件事</del>

# C++11 range-based loop

It's very convinent to use range-based for loop since C++11:

```cpp
std::vector<int> list;

for (auto &&x: list) {
    print(x + 1);
}
```

## C++20 ranges library

But sometimes, I wonder what if I need to get the index while iterating over a range?
In Python, we can use `enumerate`, `zip`, `map`, and so on.
While there is no equivalant functions in C++, until C++20 which introduced many
range operations like `std::views::transform` for `map` in Python, use it like this:

```cpp
#include <ranges>

std::vector<int> list;

for (auto &&x: list
    | std::views::transform([] (auto &&x) { return x + 1; })
    ) {
    print(x);
}
```

It will apply `+ 1` to every value in `list` before calling into the loop body.

Yeah, the `operator|` here - I'd like to call it the *pipe style*, is even more convinent than
Python's *function style* range operations. Especially when the pipe is long:

```cpp
#include <ranges>

std::vector<unique_ptr<int>> list;

for (auto &&x: list
    | std::views::transform([] (auto &&x) { return x.get(); })
    | std::views::transform([] (auto &&x) { return x + 1; })
    | std::views::reverse
    ) {
    print(x);
}
```

Of course, the function style also supported in C++20:

```cpp
#include <ranges>

std::vector<int> list;

for (auto &&x: std::views::transform([] (auto &&x) { return x + 1; }, list)) {
    print(x);
}
```


The lambda definition looks too verbose here, let's simplify it by a macro:

```cpp
#include <ranges>

#define LAMBDA1(x, ...) [&] (auto &&x) { return (__VA_ARGS__); })

std::vector<int> list;

for (auto &&x: list | std::views::transform(LAMBDA1(x, x + 1))) {
    print(x);
}
```

## Wait a miniute...

However, the `transform` or `map` isn't so useful after all.
While they don't even have `zip` and `enumerate`!!!
They are very commonly used in daily programming. Otherwise I still have to write `for (int i...` when the index is necessary.

So I decide to write my only ranges library rather than waiting for C++ standard to
support this.

# My ranges library!

First of all, we need to define a generic `range` class.

```cpp
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
```

Then, we we want to create a range for a `std::vector<int>`, we can use:

```cpp
std::vector<int> list;
auto r = range<std::vector<int>::iterator>(list.begin(), list.end());
```

To be generic, we can use `decltype` to automatically determine the iterator type of a given object:

```cpp
some_type_that_we_dont_know list;
auto r = range<decltype(list.begin())>(list.begin(), list.end());
```

## Compile-time argument deduction

But this way we will have to write `range<decltype(t.begin())>(t.begin(), t.end())` every time... Don't worry!
We can use the *complile-time argument deduction* (CTAD) feature since C++17 to automatically deduce the
`<class It>` with given rule when argument has a confirmed type:

```cpp
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
```

Then, when `range(t.begin(), t.end())` is called, it will be automatically deduced as `range<decltype(t.begin())>(t.begin(), t.end())`.

## The pipable class

Given that most transformations only takes one `range` as input, thus can support the *pipe style* syntax (excluding `zip` which requires multiple input), we'd like to define a common class for all kind of range operators, called `pipable`:

```cpp
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
```

It is able to automatically convert `f(arg)` to pipe syntax `arg | f`.

Where the following function used the `...` syntax since C++11:

```cpp
constexpr decltype(auto) operator()(auto &&...rs) const {
    return m_f(range(rs.begin(), rs.end())...);
}
```

is equivalant to:

```cpp
constexpr decltype(auto) operator()(auto &&r1, auto &&r2, and_so_on) const {
    return m_f(range(r1.begin(), r1.end()), range(r2.begin(), r2.end()), and_so_on);
}
```

with flexible number of arguments.

# Implementing map

First let's implement the easiest range operator: `map`

To do so, we need to implement our custom iterators, which requires many knowledge on what C++ iterators actually are.

## range-based for loops are 纸老虎

For example, the following C++11 range-based for loop:

```cpp
for (auto x: list) {
    print(x);
}
```

Is nothing more than a shortcut for:

```cpp
for (auto it = list.begin(); it != list.end(); ++it) {
    auto x = *it;
    print(x);
}
```

Where the `begin` and `end` are duck-typing method names for all objects to be considered iterable, so if you write:

```cpp
int not_iterable = 42;
for (auto x: not_iterable) {
    print(x);
}
```

The compiler will complain something like: error: `int` have no member named `begin`.
That's because range-based for loops are just... nothing more than shortcuts...

## Iterators are classes trying to imitate pointers

So what actually the type `it` is?

```cpp
for (auto it = list.begin(); it != list.end(); ++it) {
    auto x = *it;
    print(x);
}
```

Looking at the `*it` here, we wonder if `it` is actually a pointer, i.e. `int *`?

Not really, the real type of `list.begin()` is `vector<int>::iterator`, which is a class with these operator overloading methods defined:

- `operator*()` for getting the pointed value with `*it`
- `operator++()` for iterate to the next value with `++it`
- `operator!=()` for comparing two iterators to see if it comes to `end()`

As you can see, C++ iterators are classes trying to behave **as if it is a pointer**, for allowing algorithm reuse ability, and minimize
the effort for C programmers to adapt from C pointers to C++ iterators.

## So to implement custom iterators...

So if we want to implement our own iterators, we can simply define the above three operator overloading methods to control the behavior.

Now that we want to implement `map`, we acutally is willing the following code:

```cpp
for (auto x: list | map(func)) {
    print(x);
}
```

to be translated into:

```cpp
for (auto it = list.begin(); it != list.end(); ++it) {
    auto x = func(*it);  // changed here
    print(x);
}
```

right?

So why not just overload the `operator*()` to make it return `func(*it)` instead, while other operators remain the same.

Simple, just define a wrapper class:

```cpp
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
```

And the functor `map` that returns a range of `map_iterator`:

```cpp
static constexpr auto map(auto &&f) {
    return pipable([=] (auto &&r) {
        return range
            ( map_iterator{f, r.begin()}
            , map_iterator{f, r.end()}
            );
    });
}
```

Then test it in main function:
```cpp
int main() {
    std::vector<int> list = {1, 2, 3, 4};
    for (auto &&x: list | map([] (auto &&x) { return x + 1; })) {
        std::cout << x << std::endl;
    }
    return 0;
}
```

```
2
3
4
5
```

Succeed! We managed to make our first step in implementing ranges library ourselves!
