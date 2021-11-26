#include <iostream>

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __attribute__((visibility("default")))
#endif

extern "C" DLLEXPORT void say_hello()
{
    std::cout << "Hello Python, this is my C++ function!" << std::endl;
}

extern "C" DLLEXPORT int twice_int(int i)
{
    return i * 2;
}

extern "C" DLLEXPORT void print_str(const char *s)
{
    std::cout << "print_str: " << s << std::endl;
}

extern "C" DLLEXPORT void test_array(float *p, size_t n)
{
    for (int i = 0; i < n; i++) {
        std::cout << "arr[" << i << "] = " << p[i] << std::endl;
    }
}
