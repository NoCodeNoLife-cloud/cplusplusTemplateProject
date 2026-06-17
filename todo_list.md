# `common::data_structure` 待办清单

> 基于 `data_structure` 现状分析生成的数据结构补充计划。
>
> **当前定位：** 项目已完成大量 P0/P1 级工业级结构（R-Tree、Treiber Stack、SeqLock、ConsistentHash、
> Merkle Tree、LSM-Tree、HyperLogLog、Sparse Table、Interval Tree、T-Digest、UnionSet with Rollback 等）。
> 剩余待办聚焦于字符串索引、持久化结构、并发跳表、空间索引等方向。

---

## P1 — 高优先级

### 1. Suffix Array + LCP Array — 后缀数组

- **用途：** 字符串所有子串索引，最长公共子串、重复子串检测
- **场景：** 字符串全文索引、生物信息学
- **目录：** `data_structure/string/`（已有 AhoCorasick）
- **状态：** `[ ]` 未开始

### 2. Concurrent SkipList — 并发跳表

- **用途：** 无锁/细粒度锁跳表
- **场景：** KV 存储引擎核心结构、内存数据库索引
- **目录：** `data_structure/concurrent/`（已有 TreiberStack、MichaelScottQueue、SeqLock）
- **状态：** `[ ]` 未开始

### 3. MinHash — 最小哈希

- **用途：** 集合相似度估计，计算 Jaccard 相似度
- **场景：** 近似去重、文本相似度检测
- **目录：** `data_structure/probabilistic/`（已有 HyperLogLog、CountMinSketch、TDigest）
- **状态：** `[ ]` 未开始

---

## P2 — 中优先级

### 4. Range Tree — 范围树

- **用途：** 正交范围查询（如"矩形内所有点"）
- **场景：** 数据库查询优化、几何数据检索
- **目录：** `data_structure/spatial/`（已有 R-Tree、KDTree、Interval Tree、Quadtree）
- **状态：** `[ ]` 未开始

### 5. Persistent Vector (RRB-Tree) — 不可变向量

- **用途：** 不可变 vector，`O(log n)` 更新，近乎 `O(1)` 索引
- **场景：** 函数式编程范式、版本化数据
- **新建目录：** `data_structure/persistent/`
- **状态：** `[ ]` 未开始

### 6. HAMT (Hash Array Mapped Trie) — 哈希数组映射字典树

- **用途：** 不可变 HashMap，高性能持久化哈希表
- **场景：** 函数式编程、不可变状态管理
- **新建目录：** `data_structure/persistent/`
- **状态：** `[ ]` 未开始

### 7. Cuckoo Hashing — 布谷鸟哈希

- **用途：** 高负载因子哈希表，最坏情况 `O(1)` 查询
- **场景：** 高性能哈希表实现
- **目录：** `data_structure/hash/`（已有 ConsistentHash、RobinHoodHashMap、LRUCache）
- **状态：** `[ ]` 未开始

### 8. Lock-Free SkipList — 无锁跳表

- **用途：** 完全无锁的跳表实现
- **场景：** 多线程高并发有序集合
- **目录：** `data_structure/concurrent/`
- **状态：** `[ ]` 未开始

### 9. Suffix Automaton (SAM) — 后缀自动机

- **用途：** 字符串所有子串的 DAG，线性时间构建
- **场景：** 子串存在性/计数、字符串分析
- **目录：** `data_structure/string/`
- **状态：** `[ ]` 未开始

### 10. BK-Tree — 度量空间树

- **用途：** 拼写纠错、模糊匹配
- **场景：** 搜索引擎、输入法、词典匹配
- **目录：** `data_structure/tree/`
- **状态：** `[ ]` 未开始

### 11. Dancing Links (DLX) — 舞蹈链

- **用途：** 精确覆盖问题的算法
- **场景：** 数独求解、N 皇后、组合优化
- **目录：** `data_structure/list/`（已有 LinkedList、SkipList）
- **状态：** `[ ]` 未开始

### 12. Persistent List — 不可变链表

- **用途：** 函数式风格的 cons 操作链表
- **场景：** 纯函数式数据流
- **新建目录：** `data_structure/persistent/`
- **状态：** `[ ]` 未开始

### 13. Disjoint Set Union (Persistent) — 可持久化并查集

- **用途：** 可持久化的并查集，支持版本回溯
- **场景：** 历史版本查询、离线算法
- **新建目录：** `data_structure/persistent/`
- **状态：** `[ ]` 未开始

---

## P3 — 低优先级

### 14. Cartesian Tree — 笛卡尔树

- **用途：** 基于数组的堆式树，RMQ 与 LCA 的联系
- **目录：** `data_structure/tree/`
- **状态：** `[ ]` 未开始

### 15. Link-Cut Tree — 动态树

- **用途：** 动态树连通性问题
- **目录：** `data_structure/tree/`
- **状态：** `[ ]` 未开始

### 16. Wavelet Tree — 小波树

- **用途：** 排序序列上的 rank/select 查询
- **目录：** `data_structure/tree/`
- **状态：** `[ ]` 未开始

### 17. Radix Heap — 基数堆

- **用途：** 整数键的高效优先队列
- **目录：** `data_structure/heap/`（已有 Heap.hpp）
- **状态：** `[ ]` 未开始

### 18. Pairing Heap — 配对堆

- **用途：** 理论上效率极好的斐波那契堆替代
- **目录：** `data_structure/heap/`
- **状态：** `[ ]` 未开始

### 19. Soft Heap — 软堆

- **用途：** 允许一定误差的堆，用于近似算法
- **目录：** `data_structure/heap/`
- **状态：** `[ ]` 未开始

### 20. Van Emde Boas Tree — vEB 树

- **用途：** 整数集合的前驱/后继查询，极高效但内存大
- **目录：** `data_structure/tree/`
- **状态：** `[ ]` 未开始

### 21. Xor Linked List — 异或链表

- **用途：** 内存节省的双向链表（工程价值低）
- **目录：** `data_structure/list/`
- **状态：** `[ ]` 未开始

### 22. Unrolled Linked List — 展开链表

- **用途：** 缓存友好的链表变体
- **目录：** `data_structure/list/`
- **状态：** `[ ]` 未开始

### 23. Judy Array — Judy 数组

- **用途：** 高效整型/字符串关联数组
- **新建目录：** `data_structure/array/`
- **状态：** `[ ]` 未开始

---

## 待新建目录

以下目录尚不存在，需要在实现对应的数据结构时创建：

```
data_structure/
├── persistent/       ← 待新建（Persistent Vector、HAMT、Persistent List、Persistent DSU）
└── array/            ← 待新建（Judy Array）
```

> 其余目录（probabilistic/、spatial/、string/、concurrent/、hash/、tree/、heap/、list/ 等）均已存在。

---

## 实现风格参考

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
P1: [          ] 0 / 3  (  0%)   ← 3 项待办
P2: [          ] 0 / 10 (  0%)   ← 10 项待办
P3: [          ] 0 / 10 (  0%)   ← 10 项待办
总计: [          ] 0 / 23 (  0%)
```

> **现状：** 4 项 P1 已完成（Sparse Table、Interval Tree、T-Digest、UnionSet with Rollback），
> 剩余 **23 项**待实现：P1 3 项、P2 10 项、P3 10 项。
> 重点方向：字符串索引（Suffix Array、SAM）、持久化结构（persistent/ 系列）、并发跳表。
