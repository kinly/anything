# anything
零碎内容

## safe_destroy
- 核心是容器销毁器，注意 `raw point` 销毁仅 `delete`
- 基于更彻底的考虑，代码中有 `const_cast`
- 大部分注释由AI生成（真是强大呀）

## rank-simple
- 简易排行榜容器
- https://klysisle.space/archives/df49bf4d.html

## lfu_cache
- 区别于lru cache，根据访问次数做排序
- 只想用简单结构 `std::set` 配合全局自增量重写 operator <

## 测试中：splitter_sort
- 期望做一个简易的排行榜，用分列表的方式，底层还是用 std::map 排序
- 当前看到性能比预期低，相比裸 std::map 只是多了排名功能
- 后续持续看 & 修改
- 最终达不到期望（std::map 2倍+）的话换 skiplist
- 不过还是觉得skiplist不一定比map快（单线程场景），多线程skiplist锁的粒度理论上可以更小，所以可能更快
![image](https://github.com/user-attachments/assets/c3f3128e-47fc-48d0-bd60-e44719cf2ef0)


## nostd_source_location
- 可以在 c++20 之前（c++11 及以上）使用的编译期 source_location 信息
- 主要可用在日志模块里
- 主要是 msvc 环境的处理，gcc 里 __builtin_FILE() 等都是有的

## lru_cache
- 基于 c++20 一些新语法的尝试
- std::void_t + declval 判断是否有指定成员 https://en.cppreference.com/w/cpp/types/void_t
- 向 lambda 传递 template<class... args> 不定参模版数据，并在需要的时候使用不定参模版参数
  - 注意 lambda 的 mutable 是需要的，不然 has_on_rem 会检查失败，本想尝试 std::remove_const_t 方式，但是会报错，还没具体看
- 基于以上实现一种包装切片（切面）的行为，外部带入切面对象（如例子中的 on_rem）
- 之后在封装比如网络库、基础管理器，就不用使用纯虚接口了，切面方式更干净
- 并没有在意过细节，只是拿lru这种容器实现做测试，实际使用还需要做些修改，比如锁的范围

## dep_sort
- 拓扑排序，用于任务链问题

## string_util
- string 常用操作

## easy_allocator
- 简单版本的对象池
  - 定长：使用 std::vector
  - 不定长：使用 std::stack
- 起初是觉得定时器的 event 分配释放太频繁了
- 封装了不定参模版的 allocate，对于 c++ 更友好

## property_simple
- 一个简化的属性模型，key-value键值对 + set'ed function

## cell_helix
- 由内向外的螺旋算法，用于相对坐标点遍历

## string_simple
- string 常用的操作
  - to number
  - trim
  - split
  - stream getline handle CR LF CRLF

## tuple_helper
- tuple hash & print helper 

## endianness 
- 大小端写入、读取（基于c++的reinterpret_cast）

## easy_datetime
- 基于 std::chrono 封装的 datetime 相关
- timestamp 2 string 使用 static thread_local 避免同一秒重复做转换（缓存上次转换的s），比如在日志库使用这种
- 有一个复杂的 string 2 time_point 的函数，应对复杂的场景，实测下来比 std::get_time 方式快 1 倍，比 google 的 https://github.com/google/cctz 应该是慢的~
- 先留着把，后面有想法了做优化

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
