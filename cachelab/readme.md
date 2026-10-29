# Cache Lab

## Part A：缓存模拟器

`csim.c` 按 cache → set → line 分层，使用 LRU 替换策略。

### 函数关系

箭头表示函数调用，节点内简述各函数作用。

```mermaid
flowchart TD
    main["main：组织参数解析、模拟和汇总"]
    args["handle_argc：解析命令行选项"]
    sim["simulate_cache：打开文件、执行模拟、关闭文件"]
    summary["printSummary：打印统计并保存结果（cachelab.c）"]
    help["handle_h：帮助信息入口（待实现）"]
    open["open_trace：只读打开 trace，失败时报错退出"]
    close["fclose：关闭 trace 文件"]
    run["run_cache_simulation：遍历记录；M 执行两次访问；控制详细输出"]
    create["create_cache：动态分配组和行，初始化缓存"]
    read["read_trace_record：解析操作、地址、大小，跳过 I 和无效行"]
    access["process_access：统计 hit/miss/eviction，更新缓存和 LRU"]
    decode["abstract_tag_set_line_from_address：提取 tag、组索引和块偏移"]
    find["find_line：查找命中行、首个空行和 LRU 候选"]
    destroy["destroy_cache：依次释放行数组、组数组和缓存"]

    main --> args
    main --> sim
    main --> summary
    sim --> help
    sim --> open
    sim --> run
    sim --> close
    run --> create
    run --> read
    run --> access
    run --> destroy
    access --> decode
    access --> find
```

### 编译与测试

在 `cachelab` 目录中运行：

```bash
chmod +x test-csim
make csim && ./test-csim
```

### 实验结果

首次测试得分为 **1/27**，模拟器各项统计均为 0：

```text
                        Your simulator     Reference simulator
Points (s,E,b)    Hits  Misses  Evicts    Hits  Misses  Evicts
     0 (1,1,1)       0       0       0       9       8       6  traces/yi2.trace
     0 (4,2,4)       0       0       0       4       5       2  traces/yi.trace
     0 (2,1,4)       0       0       0       2       3       1  traces/dave.trace
     0 (2,1,3)       0       0       0     167      71      67  traces/trans.trace
     0 (2,2,3)       0       0       0     201      37      29  traces/trans.trace
     0 (2,4,3)       0       0       0     212      26      10  traces/trans.trace
     1 (5,1,5)       0       0       0     231       7       0  traces/trans.trace
     0 (5,1,5)       0       0       0  265189   21775   21743  traces/long.trace
     1

TEST_CSIM_RESULTS=1
```

修复 `getopt` 选项规则，以及返回局部数组指针的问题后，重新编译测试，全部统计与参考模拟器一致，得分为 **27/27**：

```text
                        Your simulator     Reference simulator
Points (s,E,b)    Hits  Misses  Evicts    Hits  Misses  Evicts
     3 (1,1,1)       9       8       6       9       8       6  traces/yi2.trace
     3 (4,2,4)       4       5       2       4       5       2  traces/yi.trace
     3 (2,1,4)       2       3       1       2       3       1  traces/dave.trace
     3 (2,1,3)     167      71      67     167      71      67  traces/trans.trace
     3 (2,2,3)     201      37      29     201      37      29  traces/trans.trace
     3 (2,4,3)     212      26      10     212      26      10  traces/trans.trace
     3 (5,1,5)     231       7       0     231       7       0  traces/trans.trace
     6 (5,1,5)  265189   21775   21743  265189   21775   21743  traces/long.trace
    27

TEST_CSIM_RESULTS=27
```
