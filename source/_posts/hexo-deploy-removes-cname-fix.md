---
title: hexo deploy 会删除 CNAME 解决
date: 2021-11-16 08:16:19
tags: hexo
---

# hexo deploy 时会删除 CNAME

在 GitHub 的 Settings -> Pages 页面设置了自定义域名 `archibate.top`。
就可以实现 `archibate.top` 访问而不是 `archibate.github.io` 了。

然而发现每次 `hexo d` 都会导致定制域名失效。

原理：GitHub 所谓设置自定义域名，实质是在 gh-pages 分支添加了一个叫 `CNAME` 的
文本文件，内容为你的自定义域名。
而 `hexo d` 每次都会全部重写整个 gh-pages 的内容，也就是全新的 repo 去 force push
，导致 `CNAME` 文件被覆盖。

解决：原来 hexo 的 `sources/` 文件夹内的内容都会直接出现在 gh-pages 里（下划线
开头的 `_posts` 除外）因此创建 `sources/CNAME`，内容为 `archibate.top` 即可在
下次 deploy 时候变成我的域名。

# 缺少 tags 和 categories 页面

可以看到在主页顶部有个叫 Tags 的链接，点击后跳转到 `archibate.top/tags`，出现 404 错误。

解决方法：

```bash
hexo new page tags
```

打开 `sources/tags/index.md`，修改内容为：

```md
---
title: Tag Cloud
date: .......
layout: tags
---
```

categories 同理。

其中 `layout: tags` 表示使用 `themes/freemind.386/layout/tags.ejs` 作为渲染器（？）
