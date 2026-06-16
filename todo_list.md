# `common::data_structure` 待办清单

> 基于 `data_structure` 现状分析生成的数据结构补充计划。
>
> **现状总结：** 当前 `data_structure` 擅长"教科书经典结构"（红黑树、AVL、图算法...），
> 缺的是"工业级实战结构"（LSM-Tree、HyperLogLog、Consistent Hashing、无锁队列、空间索引）。
> 下一个进化方向应从"算法竞赛选手"向"生产级基础设施"转变。

---

## P1 — 高优先级（工程价值高，P0 完成后进行）

### 1. Suffix Array + LCP Array — 后缀数组

- **用途：** 字符串所有子串索引，最长公共子串、重复子串检测
- **场景：** 字符串全文索引、生物信息学
- **新建目录：** `data_structure/string/`
- **状态：** `[ ]` 未开始

### 2. Sparse Table — 稀疏表

- **用途：** 静态 RMQ（区间最值查询），`O(1)` 查询 `O(n log n)` 构建
- **场景：** 离线查询优化、ST 表经典应用
- **目录：** `data_structure/tree/`
- **状态：** `[x]` 已完成

### 3. Interval Tree — 区间树

- **用途：** 区间重叠查询（给定一个区间，找出所有与之重叠的区间）
- **场景：** 网络包过滤、日程调度、基因比对
- **新建目录：** `data_structure/spatial/`
- **状态：** `[ ]` 未开始

### 4. Concurrent SkipList — 并发跳表

- **用途：** 无锁/细粒度锁跳表
- **场景：** KV 存储引擎核心结构、内存数据库索引
- **新建目录：** `data_structure/concurrent/`
- **状态：** `[ ]` 未开始

### 5. MinHash — 最小哈希

- **用途：** 集合相似度估计，计算 Jaccard 相似度
- **场景：** 近似去重、文本相似度检测
- **新建目录：** `data_structure/probabilistic/`
- **状态：** `[ ]` 未开始

### 6. T-Digest — 近似百分位数

- **用途：** 流式计算 P50/P95/P99 等百分位数
- **场景：** 性能监控、APM 系统（项目已有 `SystemPerformanceMonitor`）
- **新建目录：** `data_structure/probabilistic/`
- **状态：** `[x]` 已完成

### 7. UnionSet with Rollback — 可撤销并查集

- **用途：** 支持撤销（回滚）的并查集
- **场景：** 离线算法、回溯搜索、动态图连通性
- **新建目录：** `data_structure/union_find/`
- **状态：** `[ ]` 未开始

---

## P2 — 中优先级（特定的工程场景有价值）

### 8. Range Tree — 范围树

- **用途：** 正交范围查询（如"矩形内所有点"）
- **场景：** 数据库查询优化、几何数据检索
- **新建目录：** `data_structure/spatial/`
- **状态：** `[ ]` 未开始

### 9. Persistent Vector (RRB-Tree) — 不可变向量

- **用途：** 不可变 vector，`O(log n)` 更新，近乎 `O(1)` 索引
- **场景：** 函数式编程范式、版本化数据
- **新建目录：** `data_structure/persistent/`
- **状态：** `[ ]` 未开始

### 10. HAMT (Hash Array Mapped Trie) — 哈希数组映射字典树

- **用途：** 不可变 HashMap，高性能持久化哈希表
- **场景：** 函数式编程、不可变状态管理
- **新建目录：** `data_structure/persistent/`
- **状态：** `[ ]` 未开始

### 11. Cuckoo Hashing — 布谷鸟哈希

- **用途：** 高负载因子哈希表，最坏情况 `O(1)` 查询
- **场景：** 高性能哈希表实现
- **新建目录：** `data_structure/hash/`
- **状态：** `[ ]` 未开始

### 12. Lock-Free SkipList — 无锁跳表（作为 4 的补充）

- **用途：** 完全无锁的跳表实现
- **场景：** 多线程高并发有序集合
- **新建目录：** `data_structure/concurrent/`
- **状态：** `[ ]` 未开始

### 13. Suffix Automaton (SAM) — 后缀自动机

- **用途：** 字符串所有子串的 DAG，线性时间构建
- **场景：** 子串存在性/计数、字符串分析
- **新建目录：** `data_structure/string/`
- **状态：** `[ ]` 未开始

### 14. BK-Tree — 度量空间树

- **用途：** 拼写纠错、模糊匹配
- **场景：** 搜索引擎、输入法、词典匹配
- **新建目录：** `data_structure/tree/`
- **状态：** `[ ]` 未开始

### 15. Dancing Links (DLX) — 舞蹈链

- **用途：** 精确覆盖问题的算法
- **场景：** 数独求解、N 皇后、组合优化
- **新建目录：** `data_structure/list/`
- **状态：** `[ ]` 未开始

### 16. Persistent List — 不可变链表

- **用途：** 函数式风格的 cons 操作链表
- **场景：** 纯函数式数据流
- **新建目录：** `data_structure/persistent/`
- **状态：** `[ ]` 未开始

### 17. Disjoint Set Union (Persistent) — 可持久化并查集

- **用途：** 可持久化的并查集，支持版本回溯
- **场景：** 历史版本查询、离线算法
- **新建目录：** `data_structure/persistent/`
- **状态：** `[ ]` 未开始

---

## P3 — 低优先级（学术/参考价值为主）

### 18. Cartesian Tree — 笛卡尔树

- **用途：** 基于数组的堆式树，RMQ 与 LCA 的联系
- **新建目录：** `data_structure/tree/`
- **状态：** `[ ]` 未开始

### 19. Link-Cut Tree — 动态树

- **用途：** 动态树连通性问题
- **新建目录：** `data_structure/tree/`
- **状态：** `[ ]` 未开始

### 20. Wavelet Tree — 小波树

- **用途：** 排序序列上的 rank/select 查询
- **新建目录：** `data_structure/tree/`
- **状态：** `[ ]` 未开始

### 21. Radix Heap — 基数堆

- **用途：** 整数键的高效优先队列
- **新建目录：** `data_structure/heap/`
- **状态：** `[ ]` 未开始

### 22. Pairing Heap — 配对堆

- **用途：** 理论上效率极好的斐波那契堆替代
- **新建目录：** `data_structure/heap/`
- **状态：** `[ ]` 未开始

### 23. Soft Heap — 软堆

- **用途：** 允许一定误差的堆，用于近似算法
- **新建目录：** `data_structure/heap/`
- **状态：** `[ ]` 未开始

### 24. Van Emde Boas Tree — vEB 树

- **用途：** 整数集合的前驱/后继查询，极高效但内存大
- **新建目录：** `data_structure/tree/`
- **状态：** `[ ]` 未开始

### 25. Xor Linked List — 异或链表

- **用途：** 内存节省的双向链表（工程价值低）
- **新建目录：** `data_structure/list/`
- **状态：** `[ ]` 未开始

### 26. Unrolled Linked List — 展开链表

- **用途：** 缓存友好的链表变体
- **新建目录：** `data_structure/list/`
- **状态：** `[ ]` 未开始

### 27. Judy Array — Judy 数组

- **用途：** 高效整型/字符串关联数组
- **新建目录：** `data_structure/array/`
- **状态：** `[ ]` 未开始

---

## 新增目录结构总览

建议新建以下子目录来组织新增结构：

```
data_structure/
├── probabilistic/     ← 新增（T-Digest、MinHash）
├── spatial/           ← 新增（Interval Tree、Range Tree）
├── string/            ← 新增（Suffix Array、SAM）
├── concurrent/        ← 新增（Concurrent SkipList、Lock-Free SkipList）
├── persistent/        ← 新增（Persistent Vector、HAMT、Persistent List、Persistent DSU）
└── array/             ← 新增（Judy Array）
```

---

## 实现风格参考（基于项目现有风格）

```cpp
namespace common::data_structure::probabilistic
{
    // 1. 定义接口（遵循 interface/ 模式）
    template <typename T>
    class IProbabilisticFilter
    {
    public:
        virtual ~IProbabilisticFilter() = default;
        virtual void insert(const T& item) = 0;
        [[nodiscard]] virtual bool contains(const T& item) const = 0;
        [[nodiscard]] virtual double falsePositiveRate() const = 0;
        virtual void clear() = 0;
    };

    // 2. 具体实现
    template <typename T, size_t Bits = 1024, size_t Hashes = 3>
    class CuckooFilter : public IProbabilisticFilter<T> { ... };
}
```

### 关键编码要求

- 使用 `#pragma once` 统一头文件保护
- 使用 `C++26` 标准特性（concepts、`if constexpr`、折叠表达式）
- 命名空间层级：`common::data_structure::<category>::<structure>`
- `[[nodiscard]]` 标注不应丢弃的返回值
- Doxygen 风格注释 `@brief`、`@param`、`@return`、`@tparam`
- 头文件 `.hpp` + 实现文件 `.cc`
- 纯模板实现可适当放在 header 内

---

## 进度追踪

```
P0: [          ] 0 / 10 (  0%)
P1: [##        ] 2 / 7  ( 28%)
P2: [          ] 0 / 10 (  0%)
P3: [          ] 0 / 10 (  0%)
总计: [##        ] 2 / 37 (  5%)
```

> **现状：** P0 全部 10 项已完成，P1 已完成 4 项（R-Tree、Treiber Stack、SeqLock、Sparse Table）。
> 剩余 26 项待实现，涵盖字符串、空间索引、并发结构、概率型结构、持久化结构等方向。
