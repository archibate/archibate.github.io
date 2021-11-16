---
title: 安装 Claudia 主题出错
date: 2021-11-15 23:28:51
tags: hexo
---

发现一个好看的 Hexo 主题：https://github.com/Haojen/hexo-theme-Claudia

根据 README 中的指引，安装：
```bash
npm install hexo-renderer-pug
npm install hexo-renderer-sass
npm install hexo-generator-search
```

出现错误：
```
npm ERR! /home/bate/.node-gyp/16.11.1/include/node/v8-internal.h: In function ‘void v8::internal::PerformCastCheck(T*)’:
npm ERR! /home/bate/.node-gyp/16.11.1/include/node/v8-internal.h:492:38: error: ‘remove_cv_t’ is not a member of ‘std’; did you mean ‘remove_cv’?
npm ERR!   492 |             !std::is_same<Data, std::remove_cv_t<T>>::value>::Perform(data);
npm ERR!       |                                      ^~~~~~~~~~~
npm ERR!       |                                      remove_cv
npm ERR! /home/bate/.node-gyp/16.11.1/include/node/v8-internal.h:492:38: error: ‘remove_cv_t’ is not a member of ‘std’; did you mean ‘remove_cv’?
npm ERR!   492 |             !std::is_same<Data, std::remove_cv_t<T>>::value>::Perform(data);
npm ERR!       |                                      ^~~~~~~~~~~
npm ERR!       |                                      remove_cv
npm ERR! /home/bate/.node-gyp/16.11.1/include/node/v8-internal.h:492:50: error: template argument 2 is invalid
npm ERR!   492 |             !std::is_same<Data, std::remove_cv_t<T>>::value>::Perform(data);
npm ERR!       |                                                  ^
npm ERR! /home/bate/.node-gyp/16.11.1/include/node/v8-internal.h:492:63: error: ‘::Perform’ has not been declared
npm ERR!   492 |             !std::is_same<Data, std::remove_cv_t<T>>::value>::Perform(data);
npm ERR!       |                                                               ^~~~~~~
npm ERR! ../src/binding.cpp: In function ‘Nan::NAN_METHOD_RETURN_TYPE render(Nan::NAN_METHOD_ARGS_TYPE)’:
```

出错原因：`std::remove_cv_t` 是 C++14 引入的，而 `gcc` 默认为 C++11，感叹 C++ 拖后腿日常。

解决问题：
```bash
CXXFLAGS="--std=c++17" npm install hexo-renderer-sass
```
