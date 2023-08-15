# anything
零碎内容

## encoding
- gbk utf8 ~

## **handle_pool**
- 对象句柄池，给对象映射句柄
- 对象数量受 bits 分配限制，只能扩容不会缩容；可以根据实际需要调整 bits enum 值维护多个池子
- 为了尽量避免空洞，会缓存释放句柄的 bits 游标（最小值），但是导致申请句柄有向后遍历的损耗，可以用 free_list 的方式处理，不过当前代码没有这么做
- 注意不是对象池，对象的分配还是在外面的
- 示例展示了使用对象句柄池给继承链关系的游戏对象生成运行时id

## publishing_version
- python 代码，解析版本号，基于 version.template 重写一个 version.h 文件，一般在版本发布前配合发布脚本使用

## common_sum
- 新标准 `std::common_type` 的尝试，摘录的一段语法糖代码

## **attribute_tree**
- 树形结构的属性系统计划（初始模型，还在设计中）

## **game_map**
- 2D 地图相关（重点在进出视野 （镜像相交））

## sort_easy
- 简易排行榜，基于 std::map

## work_threads
- 指明工作线程的线程池
- 例如可用在开房间的游戏战斗模块，hash + mod 房间号，把网络消息推到指定线程，房间逻辑即可用单线程处理

## random_weight
- 随机数和权重随机的包装
- 权重随机实测展开式最快，空间换时间，适合一部分场景

## attribute_link
- 期望实现一个继承链关系的属性系统，不同层级有不同的属性
- 也许ecs模式是更合适的，继承链也是种选择吧
- 当前应该不是个好的写法，暂时备忘下，后面想到好的办法在做修改

## awesome_pattern.h
一些编程模式
- crtp
- pimpl
- pimpl interface
- mixin
