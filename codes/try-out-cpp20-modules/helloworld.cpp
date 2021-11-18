module;

#include <typeinfo>

export module helloworld;

export const char *hello();

const char *hello() { return "Hello C++ 20!"; }

export template <class T>
const char *nameof() {
    return typeid(T).name();
}
