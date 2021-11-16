#include <vector>
#include <cstdio>

// try remove 'noexcept' to see what happens


struct A {
    A() noexcept {
        printf("%p: A()\n", this);
    }

    A(A &&that) noexcept {
        printf("%p <- %p: A(A &&)\n", this, &that);
    }

    A(A const &that) noexcept {
        printf("%p <- %p: A(A const &)\n", this, &that);
    }

    A &operator=(A &&that) noexcept {
        printf("%p <- %p: A &operator=(A &&)\n", this, &that);
        return *this;
    }

    A &operator=(A const &that) noexcept {
        printf("%p <- %p: A &operator=(A const &)\n", this, &that);
        return *this;
    }

    ~A() noexcept {
        printf("%p: ~A()\n", this);
    }
};

int main() {
    std::vector<A> a(2);
    a.resize(3);
    return 0;
}
