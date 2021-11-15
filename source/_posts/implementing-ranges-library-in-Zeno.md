---
title: implementing ranges library in Zeno
date: 2021-11-15 11:05:58
tags:
---

# C++11 range-based loop

It's very convinent to use range-based for loop since C++11:

```cpp
std::vector<int> list;

for (auto &&x: list) {
    print(x);
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

Yeah, the `operator|` here is even more convinent than Python's function style iterator
operations, just the lambda definition looks too verbose, let's simplify it by a macro:

```cpp
#include <ranges>

#define LAMBDA1(x, ...) [&] (auto &&x) { return (__VA_ARGS__); })

std::vector<int> list;

for (auto &&x: list | std::views::transform(LAMBDA1(x, x + 1))) {
    print(x);
}
```

## Wait a miniute...

But, why they don't even have `zip`!!! Which is very commonly used in daily programming.

So I decide to write my only ranges library rather than waiting for C++ standard to
support this.

# My ranges library!

First of all, we need to define a generic `range` class, it should be implicitly castable from
any **iterable** object, including `std::vector`, `std::set`, and so on.

We should forbit ranges from being implicitly casted from **non-iterable** objects, like `int` or `std::tuple`.

So the following way must not be fesible:

```cpp
struct range {
    template <class T>
    range(T const &t) { ... }
};
```

Cause it will allow `range(int const &t)` be generated, where `int` is not iterable at all.

## Concept of iterable

So how to detect if the type `T` is **iterable**? Easy, C++ use duck-typing in
template programming, thus as long as **an object has `.begin()` and `.end()` method**, we
will consider it as an *iterable object*.

To express this, the most easiest way is to use the **concept** mechinism which is introduced in C++20:

```cpp
template <class T>
concept is_ranged = requires (T t) {
    t.begin();
    t.end();
};
```

Here, the body of `requires` statement checks if `t.begin()` and `t.end()` are **valid expressions**, i.e.
check if `t` has methods named `begin` and `end` (with no arguments).

And if the check satisfied, `is_ranged<T>` will returns `true`, otherwise `false`.

So now, we can implement the `range` constructor like this:

```cpp
struct range {
    template <class T>
        requires (is_ranged<T>)
    range(T const &t) { ... }
};
```

So that `range(int const &)` will not be valid cause `is_ranged<int>` is `false`.
Thus preventing construction of `range` from non-iterable objects.
