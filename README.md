# anything
小内容备忘

## awesome_pattern.h
一些编程模式
- crtp
- pimpl
- pimpl interface
- mixin

## attribute_link.h
- 期望实现一个继承链关系的属性系统，不同层级有不同的属性
- 也许ecs模式是更合适的，继承链也是种选择吧
- 当前应该不是个好的写法，暂时备忘下，后面想到好的办法在做修改

## random_weight.h
- 随机数和权重随机的包装
- 权重随机实测展开式最快，空间换时间，适合一部分场景

## work_threads.h
- 指明工作线程的线程池
- 例如可用在开房间的游戏战斗模块，hash + mod 房间号，把网络消息推到指定线程，房间逻辑即可用单线程处理

## sort_easy.h
- 简易排行榜，基于 std::map

## game_map.h
- 2D 地图相关（重点在进出视野 （镜像相交））

## attribute_tree.h
- 树形结构的属性系统计划（初始模型，还在设计中）

## common_sum.h
- 新标准 `std::common_type` 的尝试，摘录的一段语法糖代码

## publishing_version
- python 代码，解析版本号，基于 version.template 重写一个 version.h 文件，一般在版本发布前配合发布脚本使用
