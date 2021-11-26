#include <iostream>

extern "C" void say_hello()
{
    std::cout << "Hello Python, this is my C++ function!" << std::endl;
}

extern "C" int twice_int(int i)
{
    return i * 2;
}

extern "C" void print_str(const char *s)
{
    std::cout << "print_str: " << s << std::endl;
}

extern "C" void test_array(float *p, size_t n)
{
    for (int i = 0; i < n; i++) {
        std::cout << "arr[" << i << "] = " << p[i] << std::endl;
    }
}
