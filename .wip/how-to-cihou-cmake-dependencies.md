---
title: CMake 伺候不完全指南
date: 2021-11-20 23:54:41
tags: cmake
---

# 跨平台的构建系统

在 CMake 诞生之前，各个平台都开发了各自构建系统，例如 Windows 的 Visual Studio 系列，Linux 的 Makefile，Mac OS 的 Xcode。

然而，使用系统自带的构建系统有如下显著缺点：

- VS 每个版本都不一样，从 VS2010 到 VS2013 都需要经过一些非常复杂的转换程序，可能转换成功还没办法用。

- Makefile 相当复杂，特别是需要自己调用 `gcc -M` 和 `sed` 系列命令才能实现自动根据头文件的依赖关系决定那些 `.o` 文件需要重新构建。

- 如果在 Windows 上开发的程序用了 VS 的构建系统，则无法移植到 Linux 上…… 需要额外写一份 Makefile……还要保持他们两个的规则同步……

- 如果想使用别的 IDE 而不是 Visual Studio 进行 Windows 程序开发，就非常困难了……

- 除了 VS 和 Makefile 以外，还有 build2, ninja, premake, xmake 等，C艹的构建系统比编程语言还多系列。

后来，终端的多样化（Windows 不再是游戏操作系统 / Linux 不再是服务器专属）使得程序的可移植性变得重要起来，在编译型语言的领域，
CMake 应运而生。

CMake 的思想是，先让我们使用统一的 CMake 语法编写构建描述文件，当拿到某个特定的操作系统时，就调用相应的 **生成器 (generator)** 转换
成系统自带构建系统所能支持的格式（如 VS 的 `.vcproj` 和 make 的 `Makefile`），然后再调用本地的构建系统，真正进行项目的构建。

同时，由于 CMake 本身的语法其实比 Makefile 更丰富（VS 基于 XML 根本不是给人编辑的……），甚至可以把他看做一种脚本语言。因此使用
CMake 也可以很容易对项目进行 **配置 (configuration)**，比如，允许用户指定某个 flag，指定则定义某个宏为某个值，或者跳过某些文件
的编译等等。

> 正如 make 的指定文件名是 `Makefile`，cmake 的文件名为 `CMakeLists.txt`。

# 依赖陷阱


