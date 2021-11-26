#include <iostream>

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __attribute__((visibility("default")))
#endif

DLLEXPORT void say_hello()
{
    std::cout << "Hello Python, this is my C++ function!" << std::endl;
}

extern "C" DLLEXPORT int twice_int(int x)
{
    return x * 2;
}

extern "C" DLLEXPORT float twice_float(float x)
{
    return x * 2.f;
}

extern "C" DLLEXPORT void print_str(const char *s)
{
    printf("str is: %s\n", s);
}
