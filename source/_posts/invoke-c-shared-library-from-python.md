---
title: invoke c shared library from python
date: 2021-11-26 12:38:06
tags: python
---

# Python 调用 C 语言动态链接库里的函数

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

## Windows 不一样

然而我们试着如果用同样的配置在 Windows 上测试：

```py
import ctypes

mylib = ctypes.cdll.LoadLibrary('build\\mylib.dll')
mylib.say_hello()
```

会发现调用出错：

```py
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
