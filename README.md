# CSAPP实验记录

## Lab 1 — Data Lab: Manipulating Bits

### 实验结果

使用 `datalab-handout` 目录下的 `./btest -g` 测试所有函数，结果如下：

```text
stone@hate:~/projects/csapp-labs-stone/datalab-handout$ ./btest -g
Score   Rating  Errors  Function
 1      1       0       bitXor
 1      1       0       tmin
 1      1       0       isTmax
 2      2       0       allOddBits
 2      2       0       negate
 3      3       0       isAsciiDigit
 3      3       0       conditional
 3      3       0       isLessOrEqual
 4      4       0       logicalNeg
 4      4       0       howManyBits
 4      4       0       floatScale2
 4      4       0       floatFloat2Int
 4      4       0       floatPower2
Total points: 36/36
```

### 解题小提示

- 逻辑非 `!` 可以把 `0` 转换为 `1`，把任意非零值（包括负数）转换为 `0`。
- 判断两个值是否相等时，可以利用异或：相同的值异或结果为 `0`。
- 全 1 掩码 `0xffffffff` 与全 0 掩码只相差 `1`，可以利用这个关系，把 `0` 或 `1` 转换为按位选择所需的掩码。
- 两数做差后，可以结合差值的符号位判断大小；逻辑非 `!` 可用于判断差值是否为 `0`。
- 对负数进行算术右移时，左侧补入符号位 `1`，所以结果的最高位仍为 `1`。

详细的解题注释和代码位于 `datalab-handout/bits.c`，写在各函数定义后的函数体开头注释中。

## Lab 2 — Bomb Lab: Defusing a Binary Bomb

### 实验结果

在 `bomb` 目录下运行 `./bomb`，六个阶段全部通过，运行记录如下：

```text
stone@hate:~/projects/csapp-labs-stone/bomb$ ./bomb
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Border relations with Canada have never been better.
Phase 1 defused. How about the next one?
1 2 4 8 16 32
That's number 2.  Keep going!
0 207
Halfway there!
7 0
So you got that one.  Try this one.
IONEFG
Good work!  On to the next...
4 3 2 1 6 5
Congratulations! You've defused the bomb!
```

### 答案限制规则

以下规则对应本仓库的 `bomb` 程序；部分阶段存在多组合法答案，上面的运行记录只展示本次使用的一组。

- **Phase 1：字符串完全匹配。** 输入必须与 `Border relations with Canada have never been better.` 完全一致，包括大小写、空格和句号；仅长度相同不能通过。
- **Phase 2：倍增数列。** 读取六个整数，第一个必须是 `1`，后一个必须是前一个的两倍，因此为 `1 2 4 8 16 32`。
- **Phase 3：分支编号与常量匹配。** 读取两个整数，第一个必须在 `0～7`，用作跳转表索引；第二个必须等于该分支设置的常量。例如本次选择编号 `0`，对应常量 `207`，所以输入 `0 207`。其他编号对应其他合法组合。
- **Phase 4：递归返回值与第二个整数均为零。** 读取两个整数，第一个必须在 `0～14`，并使 `func4(第一个整数, 0, 14)` 返回 `0`；第二个必须是 `0`。本程序满足条件的第一个整数有 `0、1、3、7`，本次使用 `7 0`。
- **Phase 5：六个字符查表后得到目标字符串。** 输入长度必须为六个字节；每个字节与 `0xf` 按位与，取低四位作为表 `maduiersnfotvbyl` 的索引，查表结果必须依次组成 `flyers`。因此六个字节的低四位依次为 `9、15、14、5、6、7`；本次使用 `IONEFG`，相同低四位的其他字符组合也可能通过。
- **Phase 6：按节点数值降序重新连接链表。** 六个整数必须是 `1～6` 的一个排列。程序将每个整数变成 `7 - 输入值`，作为从 `1` 开始计数的原链表节点位置编号；按这些编号选择并连接节点后，要求每个节点的数值大于等于下一节点。这里降序的是节点数值，不是输入或编号。本程序节点数值降序对应编号 `3 4 5 6 1 2`，反推输入为 `4 3 2 1 6 5`。

详细解题批注位于 `bomb/bomb_disassembly.txt` 各条汇编指令后的 `#` 注释中，答案保存在 `bomb/answer.txt`，实验与 GDB 命令速查见 `bomb/README.md`。

## Lab 3 — Attack Lab: Understanding Buffer Overflow Bugs

### 实验结果

在 `attacklab` 目录使用 `phase1.txt`～`phase5.txt` 中的输入进行本地验证，五个阶段全部通过，目标程序均正常退出。

| 阶段    | 目标程序    | 方法与目标                                        | 结果 |
| ------- | ----------- | ------------------------------------------------- | ---- |
| Phase 1 | `ctarget` | 覆盖返回地址，执行 `touch1()`                   | PASS |
| Phase 2 | `ctarget` | 注入代码，将整数 cookie 传给 `touch2`           | PASS |
| Phase 3 | `ctarget` | 注入代码，将 cookie 字符串地址传给 `touch3`     | PASS |
| Phase 4 | `rtarget` | 使用 ROP，将整数 cookie 传给 `touch2`           | PASS |
| Phase 5 | `rtarget` | 使用 ROP，计算 cookie 字符串地址并传给 `touch3` | PASS |

运行命令及关键输出如下，省略了重复的 cookie 提示和完整输入字节记录：

```text
$ ./hex2raw < phase1.txt | ./ctarget -q
Type string:Touch1!: You called touch1()
Valid solution for level 1 with target ctarget
PASS: Would have posted the following:

$ ./hex2raw < phase2.txt | ./ctarget -q
Type string:Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target ctarget
PASS: Would have posted the following:

$ ./hex2raw < phase3.txt | ./ctarget -q
Type string:Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target ctarget
PASS: Would have posted the following:

$ ./hex2raw < phase4.txt | ./rtarget -q
Type string:Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target rtarget
PASS: Would have posted the following:

$ ./hex2raw < phase5.txt | ./rtarget -q
Type string:Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target rtarget
PASS: Would have posted the following:
```

`-q` 表示仅在本地验证，不向课程服务器提交。输出中的 level 是目标程序内部的验证等级，因此 Phase 4、Phase 5 分别显示 `rtarget` 的 level 2、level 3。

详细的栈布局、注入代码、ROP 链与调试过程见 [Attack Lab 实验记录](attacklab/readme.md)，各阶段输入保存在 `attacklab/phase1.txt`～`attacklab/phase5.txt`，反汇编记录位于 `attacklab/ctarget_disassembly.txt` 和 `attacklab/rtarget_disassembly.txt`。

## Lab 4 — Architecture Lab: Optimizing the Performance of a Pipelined Processor

### 实验结果

以下结果整理自 [Architecture Lab 实验记录](archlab-handout/readme.md)。

| 部分   | 实现内容                                                                     | 验证结果                                                                                   |
| ------ | ---------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------ |
| Part A | Y86-64 迭代求和 `sum.ys`、递归求和 `rsum.ys`、复制与异或校验 `copy.ys` | 三个程序均正常停止；本次输入的返回值均为 `0xcba`，复制结果正确                           |
| Part B | 在 `seq-full.hcl` 中实现 `iaddq`                                         | `asumi.ys` 验证成功，16 个示例全部通过，含 `iaddq` 的 932 项 ISA 检查全部通过          |
| Part C | 优化 `ncopy.ys` 与 `pipe-full.hcl`                                       | YIS 和 PIPE 正确性测试均为 68/68，平均 CPE 为 **7.61**，性能得分 **57.8/60.0** |

Part C 采用八路循环展开、`iaddq`、加载到存储转发、BTFNT 分支预测，以及按余数定位的连续尾部处理。主要性能变化如下：

| 优化阶段                  |    Average CPE |            性能得分 |
| ------------------------- | -------------: | ------------------: |
| 初次指令重排              |          14.03 |            0.0/60.0 |
| 二路展开并外提常数        |          10.01 |            9.8/60.0 |
| 八路展开                  |           8.83 |           33.4/60.0 |
| PIPE 支持并使用 `iaddq` |           8.54 |           39.2/60.0 |
| 连续尾部处理              |           7.81 |           53.8/60.0 |
| BTFNT 分支预测            |           7.71 |           55.8/60.0 |
| 最终尾部定位优化          | **7.61** | **57.8/60.0** |

最终版本在 `archlab-handout/sim/pipe` 中的验证命令及关键输出：

```text
$ ./correctness.pl -q -f ncopy.ys
Simulating with instruction set simulator yis
68/68 pass correctness test

$ ./correctness.pl -q -p -f ncopy.ys
Simulating with pipeline simulator psim
68/68 pass correctness test

$ ./benchmark.pl -f ncopy.ys
64      403     6.30
Average CPE     7.61
Score   57.8/60.0
```

其中 `64 403 6.30` 表示长度为 64 时耗费 403 个周期，CPE 为 6.30；平均 CPE 7.61 是长度 1～64 各自 CPE 的算术平均。57.8/60.0 为 Part C 性能得分。

源码位于 `archlab-handout/sim/misc/{sum,rsum,copy}.ys`、`archlab-handout/sim/seq/seq-full.hcl`、`archlab-handout/sim/pipe/{ncopy.ys,pipe-full.hcl}`；完整测试记录和优化过程见 [详细笔记](archlab-handout/readme.md)。

## Lab 5 — Cache Lab: Understanding Cache Memories

### 实验结果

Part A 的缓存模拟器使用 LRU 替换策略，已有测试记录为 **27/27**；Part B 的矩阵转置在本次三种尺寸测试中均正确，miss 数全部达到满分要求，对应性能得分 **26/26**。

| 部分             | 实现内容                                                            | 验证结果                        |
| ---------------- | ------------------------------------------------------------------- | ------------------------------- |
| Part A           | 在 `csim.c` 中解析访存记录，模拟缓存命中、未命中、驱逐和 LRU 替换 | 各项统计与参考模拟器一致，27/27 |
| Part B：32 × 32 | 8 × 8 分块，先把一行的 8 个元素读入临时变量，再写 B                | correctness=1，288 miss，8/8    |
| Part B：64 × 64 | 8 × 8 块内分成四个区域，借用 B 暂存，分三个阶段完成转置            | correctness=1，1228 miss，8/8   |
| Part B：61 × 67 | 16 × 16 分块，检查矩阵边界                                         | correctness=1，1993 miss，10/10 |

Part B 的测试缓存参数为 `s=5, E=1, b=5`，即 32 个组、每组一条缓存行、每块 32 字节，总数据容量为 1 KiB。三种尺寸的满分要求分别是 miss 不超过 **300、1300、2000**。

在 `cachelab` 目录编译并测试，本次提交函数的关键输出如下：

```text
$ make
$ ./test-trans -M 32 -N 32
Summary for official submission (func 0): correctness=1 misses=288
TEST_TRANS_RESULTS=1:288

$ ./test-trans -M 64 -N 64
Summary for official submission (func 0): correctness=1 misses=1228
TEST_TRANS_RESULTS=1:1228

$ ./test-trans -M 61 -N 67
Summary for official submission (func 0): correctness=1 misses=1993
TEST_TRANS_RESULTS=1:1993
```

### 优化思路

- **先分块，缩小搬运范围。** 尽快用完当前小块涉及的内存块，减少处理下一行时重新加载 B 的次数。
- **先读满，再写出。** 用 8 个临时变量保存 A 当前行的元素，写 B 时即使驱逐了这个 A 块，也不用回头读取。
- **按缓存映射安排写入顺序。** 64 × 64 中，B 相隔 4 行的块会映射到同一组。先完成上方行，再写下方对应行，避免反复互相驱逐。
- **根据尺寸选择策略。** 61 × 67 的行首不总与内存块边界对齐，采用带边界检查的 16 × 16 分块，并通过实测选择大小。

源码见 [缓存模拟器](cachelab/csim.c) 和 [矩阵转置](cachelab/trans.c)；地址位划分、逐次 hit/miss 分析、三阶段搬运及完整实验数据见 [Cache Lab 详细笔记](cachelab/readme.md)。

## Lab 6 — Shell Lab: Writing Your Own Unix Shell

### 实验结果

在 Linux 上重新编译 `tsh`，使用实验提供的 `sdriver.pl`，逐条运行 `trace01.txt`～`trace16.txt`，并与参考程序 `tshref` 对照：**本轮 16/16 条测试均符合输出及进程状态要求**。

| 测试范围    | 验证内容                             | 本地对照结果                       |
| ----------- | ------------------------------------ | ---------------------------------- |
| trace01–05 | EOF、quit、前后台作业及 jobs         | 5/5 一致                           |
| trace06–08 | 向前台作业转发 SIGINT、SIGTSTP       | 3/3 一致                           |
| trace09–10 | bg、fg 作业控制                      | 2/2 一致                           |
| trace11–13 | 向整个进程组转发信号并恢复停止的进程 | 3/3 一致，mysplit 进程状态符合预期 |
| trace14     | 命令与参数错误处理                   | 一致                               |
| trace15     | 综合作业控制                         | 一致                               |
| trace16     | 子进程自行触发停止或终止信号         | 一致                               |

比较时允许 PID 不同；trace11～13 中的 `/bin/ps a` 输出按本轮 `mysplit` 进程的运行状态核对。其余输出与参考程序一致，各测试驱动均正常结束。

源码见 [tsh.c](shelllab/tsh.c)；验证环境、全部 16 条测试及参考程序的命令行输出见 [Shell Lab 完整实验记录](shelllab/readme.md)。

## Lab 7 — Malloc Lab: Writing a Dynamic Storage Allocator

### 实验结果

当前采用隐式空闲链表、首次适配查找、块拆分及相邻空闲块立即合并，并支持 `realloc` 调整大小与数据搬运。根据本次本地运行记录，**两个 short 测试和全部 11 个完整 trace 均通过正确性检查**；完整测试性能得分为 **67/100**。

| 测试集             | 正确性结果 | 空间利用率 | 吞吐量     | 性能得分 |
| ------------------ | ---------- | ---------- | ---------- | -------- |
| `short1-bal.rep` | 通过       | 66%        | 60000 Kops | 80/100   |
| `short2-bal.rep` | 通过       | 89%        | 60000 Kops | 94/100   |
| 完整 11 个 trace   | 11/11 通过 | 77%        | 316 Kops   | 67/100   |

在 `malloclab` 目录编译并运行，完整测试的汇总输出如下：

```text
$ make
$ ./mdriver -v -f short1-bal.rep
$ ./mdriver -v -f short2-bal.rep
$ ./mdriver -v -t ./traces/

Results for mm malloc:
trace  valid  util     ops      secs  Kops
Total          77%  112372  0.355998   316

Perf index = 46 (util) + 21 (thru) = 67/100
```

完整测试共执行 112372 次操作。当前主要耗时集中在 `binary-bal.rep`、`binary2-bal.rep`，吞吐量分别约为 160、100 Kops；隐式链表首次适配的反复扫描是后续优化方向。short 测试各只有 12 次操作，计时较短，性能比较以完整测试为主；吞吐量随机器和运行环境变化。

源码见 [mm.c](malloclab/mm.c)；块布局、初始化、扩堆、分配与释放、`realloc` 处理及待完善项见 [Malloc Lab 实验记录](malloclab/readme.md)。

## Lab 8 — Proxy Lab: Writing a Caching Web Proxy

### 实验结果

已完成顺序代理、多线程并发和内存缓存，官方 `driver.sh` 最终测试为 **70/70**。

| 阶段             | Basic | Concurrency | Cache | 总分            |
| ---------------- | ----- | ----------- | ----- | --------------- |
| Part 1：顺序代理 | 40/40 | 0/15        | 0/15  | 40/70           |
| Part 2：并发代理 | 40/40 | 15/15       | 0/15  | 55/70           |
| Part 3：缓存代理 | 40/40 | 15/15       | 15/15 | **70/70** |

每个连接由独立的 detached 线程处理；缓存采用链表、近似 LRU 淘汰和信号量实现的读者—写者同步，允许多个线程同时读取。

在仓库根目录执行：

```bash
cd proxylab
make
make -C tiny
NO_PROXY= no_proxy= ./driver.sh
```

清空 `NO_PROXY` 是为了确保本地请求经过代理。官方脚本做了执行权限、`ss` 端口检测和 Python 3 的环境适配，评分逻辑未改动。

最终一次官方测试的真实输出如下：

```text
*** Basic ***
Starting tiny on 24725
Starting proxy on 9551
1: home.html
   Fetching ./tiny/home.html into ./.proxy using the proxy
   Fetching ./tiny/home.html into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
2: csapp.c
   Fetching ./tiny/csapp.c into ./.proxy using the proxy
   Fetching ./tiny/csapp.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
3: tiny.c
   Fetching ./tiny/tiny.c into ./.proxy using the proxy
   Fetching ./tiny/tiny.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
4: godzilla.jpg
   Fetching ./tiny/godzilla.jpg into ./.proxy using the proxy
   Fetching ./tiny/godzilla.jpg into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
5: tiny
   Fetching ./tiny/tiny into ./.proxy using the proxy
   Fetching ./tiny/tiny into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
Killing tiny and proxy
basicScore: 40/40

*** Concurrency ***
Starting tiny on port 30793
Starting proxy on port 18292
Starting the blocking NOP server on port 9481
Trying to fetch a file from the blocking nop-server
Fetching ./tiny/home.html into ./.noproxy directly from Tiny
Fetching ./tiny/home.html into ./.proxy using the proxy
Checking whether the proxy fetch succeeded
Success: Was able to fetch tiny/home.html from the proxy.
Killing tiny, proxy, and nop-server
concurrencyScore: 15/15

*** Cache ***
Starting tiny on port 22191
Starting proxy on port 26042
Fetching ./tiny/tiny.c into ./.proxy using the proxy
Fetching ./tiny/home.html into ./.proxy using the proxy
Fetching ./tiny/csapp.c into ./.proxy using the proxy
Killing tiny
Fetching a cached copy of ./tiny/home.html into ./.noproxy
Success: Was able to fetch tiny/home.html from the cache.
Killing proxy
cacheScore: 15/15

totalScore: 70/70
```

源码见 [proxy.c](proxylab/proxy.c)、[cache.c](proxylab/cache.c) 和 [cache.h](proxylab/cache.h)；函数结构图及各阶段完整测试输出见 [Proxy Lab 实验记录](proxylab/README.md)。
