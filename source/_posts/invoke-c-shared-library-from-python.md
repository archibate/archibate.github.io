---
title: Python 调用 C 语言动态链接库里的函数
date: 2021-11-26 12:38:06
tags: python
---

源代码在: https://github.com/archibate/archibate.github.io/tree/master/codes/invoke-c-shared-library-from-python

Python 支持导入任意由 C 语言编写的动态链接库（`.so` 或者 `.dll`），并调用其中的函数。
这其中又有哪些坑呢，今天就让我们一探究竟。

# 符号导出机制

针对不同的系统，分类讨论：

## Linux 很简单

首先用 CMake 创建一个 shared library 目标：

```cmake
# CMakeLists.txt
add_library(mylib SHARED mylib.c)
```

C 语言源代码如下：

```cpp
// mylib.c
#include <stdio.h>

void say_hello() {
    printf("Hello, world!\n");
}
```

```bash
cmake -B build
cmake --build build
```

编译后会得到 `build/libmylib.so` 。

然后在 Python 脚本里写：

```py
import ctypes

mylib = ctypes.cdll.LoadLibrary('build/libmylib.so')
mylib.say_hello()
```

成功打印：

```py
Hello, world!
```

## Windows 不一样

然而我们试着如果用同样的配置在 Windows 上测试：

```py
import ctypes

mylib = ctypes.cdll.LoadLibrary('build\\mylib.dll')
mylib.say_hello()
```

会发现调用出错：

```bash
AttributeError: build\mylib.dll: undefined symbol: say_hello
```

这是什么原因呢？

原来 Windows 的设计为了安全，默认不会把 DLL 的所有符号导出，需要这样写：

```c
// mylib.c
#include <stdio.h>

__declspec(dllexport) void say_hello() {
    printf("Hello, world!\n");
}

void this_func_wont_export() {}
```

需要导出的，前面加上 `__declspec(dllexport)`，不希望导出的，就不加。
如果想要和 Linux 一样默认全部导出，也可以在 CMake 里这样指定：

```cmake
# CMakeLists.txt
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(mylib SHARED mylib.c)
```

## Linux 也可以默认不导出

反过来，Linux 也可以修改成默认不导出，只有指定的符号才导出：

```cmake
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

add_library(mylib SHARED mylib.c)
```

然后像这样指定前缀：

```c
// mylib.c
#include <stdio.h>

__attribute__((visibility("default"))) void say_hello() {
    printf("Hello, world!\n");
}

void this_func_wont_export() {}
```

我们可以用一个宏来统一 Windows 和 Linux 的导出指定前缀：

```c
#ifdef _WIN32  // 如果在 Windows 上
#define DLLEXPORT __declspec(dllexport)
#else          // 否则在 Unix 类系统上
#define DLLEXPORT __attribute__((visibility("default")))
#endif

DLLEXPORT void say_hello() {
    printf("Hello, world!\n");
}
```

# C++ 大不一样

然而，如果我们用的不是 C 语言，而是 C++，还是会出现符号找不到的问题：

```bash
AttributeError: build/mylib.so: undefined symbol: say_hello
```

这是为什么呢？我们来用 Linux 下的 `nm` 小工具分析一下生成的动态链接库。

用 C 语言编译：
```bash
$ nm build/libmylib.so | grep -w T
0000000000001109 T say_hello
```

用 C++ 编译：
```bash
$ nm build/libmylib.so | grep -w T
0000000000001139 T _Z9say_hellov
```

可以看到 C++ 中原本叫 `say_hello` 的函数变成了一个奇怪的名字： `_Z9say_hellov`
这是为什么呢？

## C++ 函数名重组机制

原来是 C++ 为了实现函数的重载和名字空间等特性，对函数名对应的符号进行了一些魔改，
C++ 魔改的符号都以 `_Z` 开头，后面紧跟着一个数字，表示接下来的符号长度，这里
`say_hello` 的字符串长度为 9，因此是 `_Z9`，然后 `v` 表示函数的参数是 void，也就
是没有参数。

解决方法是要么直接在 Python 里写重组后的符号名：

```py
import ctypes

mylib = ctypes.cdll.LoadLibrary('build/libmylib.so')
mylib._Z9say_hellov()
```

要么在 C++ 源文件里使用 `extern "C"` 声明为 C 兼容函数（但是没法重载了）：

```cpp
extern "C" DLLEXPORT void say_hello() {
    printf("Hello, world!\n");
}
```

个人推荐后面一种方案。

# 读取哪一个文件

这里我们硬编码了 `build\\mylib.dll` 等路径，导致无法跨平台。
可以让 Python 运行时动态判断当前是什么系统，读取不同的文件路径和扩展名。

```py
import ctypes
import sys
import os

def load_library(path, name):
    if sys.platform == 'win32':       # *.dll
        return ctypes.cdll.LoadLibrary(os.path.join(path, name + '.dll'))
    elif sys.platform == 'linux':     # lib*.so
        return ctypes.cdll.LoadLibrary(os.path.join(path, 'lib' + name + '.so'))
    elif sys.platform == 'darwin':    # lib*.dylib
        return ctypes.cdll.LoadLibrary(os.path.join(path, 'lib' + name + '.dylib'))
    else:
        raise ImportError('Unsupported platform: ' + sys.platform)

mylib = load_library('build', 'mylib')
```

这里我们用了 `sys.platform` 判断当前操作系统，`os.path.join` 在 Unix 类系统上是 `'/'.join`，
Windows 上是 `'\\'.join`。`path` 用作读取文件所在的目录，`name` 和 CMake 的目标名相同。

# 函数参数与返回

让我们定义几个带参数的函数作为测试：

```cpp
extern "C" DLLEXPORT int twice_int(int x) {
    return x * 2;
}

extern "C" DLLEXPORT float twice_float(float x) {
    return x * 2.f;
}

extern "C" DLLEXPORT void print_str(const char *s) {
    printf("str is: %s\n", s);
}
```

Python 调用的 C 函数可以有参数和返回值，不过 Python 实际并不知道有几个参数，分别是什么类型，
所以需要我们自己去指定参数和返回值的类型（如果不指定，默认参数全部为 int，返回值也为 int）：

```py
mylib.twice_int.argtypes = [ctypes.c_int]
mylib.twice_int.restype = ctypes.c_int
mylib.twice_float.argtypes = [ctypes.c_float]
mylib.twice_float.restype = ctypes.c_float
mylib.print_str.argtypes = [ctypes.c_char_p]
mylib.print_str.restype = None

print(mylib.twice_int(21))       # 42
print(mylib.twice_float(3.14))   # 6.28
mylib.print_str(b'Hello, C++!')
```

```bash
$ python main.py
42
6.28000020980835
str is: Hello, C++!
```

## 传递 NumPy 数组

有时候我们需要把一个 Python 端的数组传入/传出 C 语言的部分，可以传一个指针 + 一个大小来表示数组：

```py
mylib.test_array.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
mylib.test_array.restype = None

import numpy as np

arr = np.random.rand(32).astype(np.float32)
mylib.test_array(arr.ctypes.data, arr.shape[0])
```

```cpp
extern "C" DLLEXPORT void test_array(float *base, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        printf("%ld: %f\n", i, base[i]);
    }
}
```
