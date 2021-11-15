---
title: implementing ranges library from scratch
date: 2021-11-15 11:05:58
tags: cpp
---

# 手把手教你在 C++17 中从零开始实现 ranges 库

<del>关于为什么 C++20 引入了 ranges 库却没有 zip 还要用户自己实现这件事</del>

So in this post, you will learn how to write a ranges library from scratch in C++17,
with some useful functionality that doesn't exists in the C++20 library `<ranges>`,
like `enumerate` or `zip`.

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
    template <class ...Rs>
    constexpr decltype(auto) operator()(Rs &&...rs) const {
        return m_f(range(rs.begin(), rs.end())...);
    }

    // pipe style: range | map(func)
    template <class R>
    friend constexpr decltype(auto) operator|(R &&r, pipable const &self) {
        return self(std::forward<decltype(r)>(r));
    }
};
```

It is able to automatically convert `f(arg)` to pipe syntax `arg | f`.

Where the following function used the `...` syntax since C++11:

```cpp
template <class ...Rs>
constexpr decltype(auto) operator()(Rs &&...rs) const {
    return m_f(range(rs.begin(), rs.end())...);
}
```

is equivalant to:

```cpp
template <class R1, class R2, and_so_on>
constexpr decltype(auto) operator()(R1 &&r1, R2 &&r2, and_so_on) const {
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

    // using `decltype(auto)` (since C++14) instead of `auto` so that
    // even if `m_func` returns a reference, it automatically becames
    // `auto &` rather than dereferencing that...
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
```

And the functor `map` that returns a range of `map_iterator`:

```cpp
template <class F>
static constexpr auto map(F &&f) {
    return pipable([=] (auto &&r) {
        return range
            ( map_iterator{f, r.begin()}
            , map_iterator{f, r.end()}
            );
    });
}
```

> Here we used `map_iterator{...}` rather than `map_iterator(...)`
> so that we don't have to write the constructor ourselves but ask
> the compiler to assign its members with default order (since C++11).

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

Compile and run it:
```
$ g++ -std=c++20 a.cpp && ./a.out
2
3
4
5
```

Succeed! We managed to make our first step in implementing ranges library ourselves!

# What's next?

Next, we will go ahead to implement the `enumerate` and `zip` as well with the same
technique we learnt from implementing the `map`.

## Implement enumerate

Now that we want to implement `enumerate`, we acutally is willing the following code:

```cpp
for (auto [x, y]: list | enumerate(func)) {
    print(x, y);
}
```

to be translated into:

```cpp
size_t index = 0;
for (auto it = list.begin(); it != list.end(); ++it, ++index) {
    auto [x, y] = std::pair(index, *it);
    print(x, y);
}
```

right? 

> Here, the `auto [x, y] = ...` is the **structual binding** syntax since C++17,
> and `...` can be a `std::pair`, or `std::tuple`, or any thing unpackable, we will
> take a deep look into this later.

So now we need to overload `operator*()` to make it return a pair of values, whose
first value is the index, which is incresed during `operator++()`, like this:

```cpp
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
```

And since the enumerate takes no `Func` as input like `map` does, we can
simply define `enumerate` as a global variable:

```
static constexpr auto enumerate = pipable([] (auto &&r) {
    return range
        ( enumerate_iterator{r.begin()}
        , enumerate_iterator{r.end()}
        );
});
```

> The `static` here make sure that the symbol doesn't conflict when the header
> is being included for multiple times in a single project.

Test it again:
```cpp
int main() {
    std::vector<int> list = {1, 2, 3, 4};
    for (auto &&[x, y]: list | enumerate) {
        std::cout << x << ' ' << y << std::endl;
    }
    return 0;
}
```

Or use the function style if you like:
```cpp
int main() {
    std::vector<int> list = {1, 2, 3, 4};
    for (auto &&[x, y]: enumerate(list)) {
        std::cout << x << ' ' << y << std::endl;
    }
    return 0;
}
```

It should outputs:
```
0 1
1 2
2 3
3 4
```

Worked! Congrats on being Pythonic in modern C++ programming :)
Go ranges and no more `for (int i = 0; ...` bolierplates!

## What about zip?

Homework time! Please try out what you've learnt by implement the zip
yourself, to test and solidify your new skill :)

For example:
```cpp
int main() {
    std::vector<int> list1 = {1, 2, 3, 4};
    std::vector<int> list2 = {3, 3, 9, 9};
    for (auto &&[x, y]: zip(list1, list2)) {
        std::cout << x << ' ' << y << std::endl;
    }
    return 0;
}
```
should outputs:
```
1 3
2 3
3 9
4 9
```

I also prepared some challenge for curious readers, here they goes:

### challenge A

* Make `zip` resulting range size to be the **minimum of all input ranges**.

For example:
```cpp
int main() {
    std::vector<int> list1 = {1, 2, 3, 4, 5, 6, 7};
    std::vector<int> list2 = {3, 9};
    for (auto &&[x, y]: zip(list1, list2)) {
        std::cout << x << ' ' << y << std::endl;
    }
    std::cout << "===" << std::endl;
    for (auto &&[x, y]: zip(list2, list1)) {  // order reversed!
        std::cout << x << ' ' << y << std::endl;
    }
    return 0;
}
```
should outputs:
```
1 3
2 9
===
3 1
9 2
```

### challenge B

* Make `zip` takes `n` number of ranges as input, while `operator*()`
returns a **`std::tuple` with size `n`**.

For example:
```cpp
int main() {
    std::vector<int> list1 = {1, 2, 3, 4};
    std::vector<int> list2 = {3, 3, 9, 9};
    std::vector<float> list3 = {3.14f, 2.718f, 1.414f, 0.618f};
    for (auto &&[x, y, z]: zip(list1, list2, list3)) {
        std::cout << x << ' ' << y << std::endl;
    }
    return 0;
}
```
should outputs:
```
1 3 3.14
2 3 2.718
3 9 1.414
4 9 0.618
```

> Yes, the Python `zip` can pass above two 'challenge' :)

# Finally...

You may 'submit' the 'homework' via 评论区, or GitHub, or any
other way that fesible. Submitting this homework won't affect
anyone's GPA or KPI, but this one is *just for fun*!

Below is the final source code of this tutorial, your homework
may be either based on it or completely from scratch:

```cpp
// compile with -std=c++17
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


int main() {
    std::vector<int> list = {1, 2, 3, 4};
    for (auto &&x: list | map([] (auto &&x) { return x + 1; })) {
        std::cout << x << std::endl;
    }
    for (auto &&[x, y]: enumerate(list)) {
        std::cout << x << ' ' << y << std::endl;
    }
    // TODO: implement zip...
    return 0;
}
```

Also checkout my [personal website](archibate.top) [[国内镜像站]](archibate.gitee.io)
where this post were uploaded, Zhihu will also be uploaded synchronously.
