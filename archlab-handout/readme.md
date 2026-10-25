# Y86-64 指令速查

用于 Arch Lab。`q` 表示操作 64 位（8 字节）数据；双操作数通常按 **源、目的** 的顺序书写。

## 1. 寄存器与基本语法

共有 15 个 64 位通用寄存器，没有 `%r15`：

```text
%rax %rcx %rdx %rbx %rsp %rbp %rsi %rdi
%r8  %r9  %r10 %r11 %r12 %r13 %r14
```

- `%rsp`：栈顶指针，栈向低地址增长。
- 按实验常用调用约定，`%rdi`、`%rsi`、`%rdx` 依次传递前三个参数，`%rax` 保存返回值；这是约定，不是指令的硬性要求。
- `$10`：立即数 10；`%rax`：寄存器中的值；`8(%rdi)`：地址为 `%rdi + 8` 的内存。
- `label:` 定义标签；`#` 后面是注释。
- 内存寻址使用 `位移(基址寄存器)`，例如 `(%rdi)`、`8(%rdi)`；不支持 x86-64 的比例变址形式。

## 2. 数据传送

记忆：`i` = 立即数，`r` = 寄存器，`m` = 内存。

| 指令                 | 示例                     | 含义                            |
| -------------------- | ------------------------ | ------------------------------- |
| `rrmovq rA, rB`    | `rrmovq %rax, %rbx`    | 把 `%rax` 的值复制到 `%rbx` |
| `irmovq V, rB`     | `irmovq $8, %rcx`      | 把立即数 8 写入 `%rcx`        |
| `rmmovq rA, D(rB)` | `rmmovq %rax, 8(%rdi)` | 把 `%rax` 的 8 字节值写入内存 |
| `mrmovq D(rB), rA` | `mrmovq 8(%rdi), %rax` | 从内存读取 8 字节到 `%rax`    |

标签也可以作为地址常量：`irmovq list, %rdi` 把 `list` 的地址放入 `%rdi`，不会读取该地址处的数据。

这些指令不修改条件码。基础 Y86-64 不支持直接把立即数写入内存，也不支持直接在两处内存之间传送。

## 3. 算术与逻辑运算

以下指令都使用两个寄存器，将结果写回第二个寄存器，并更新条件码。

| 指令            | 运算含义                       |
| --------------- | ------------------------------ |
| `addq rA, rB` | `rB = rB + rA`               |
| `subq rA, rB` | `rB = rB - rA`，注意减法顺序 |
| `andq rA, rB` | `rB = rB & rA`，按位与       |
| `xorq rA, rB` | `rB = rB ^ rA`，按位异或     |

常用写法：

```asm
xorq %rax, %rax       # 清零 %rax，同时更新条件码
andq %rdi, %rdi       # 保持 %rdi 不变，根据其值设置条件码
irmovq $8, %rcx
addq %rcx, %rdi       # 指针前进 8 字节
```

基础 Y86-64 没有 `cmpq`、`testq`、`incq`、`decq`、乘除和移位指令，也不能直接写 `addq $8, %rdi`。

## 4. 条件码、跳转与条件传送

条件码来自最近一次设置条件码的运算：

| 条件码 | 含义                                                |
| ------ | --------------------------------------------------- |
| `ZF` | 运算结果为 0 时置 1                                 |
| `SF` | 运算结果的最高位为 1 时置 1，表示结果为负           |
| `OF` | 有符号加减法溢出时置 1；`andq`、`xorq` 将其清零 |

`jmp label` 无条件跳转。条件跳转和条件传送使用相同的判断规则：

| 跳转    | 条件传送   | 条件                          | 含义（在 `subq rA, rB` 之后比较原值） |
| ------- | ---------- | ----------------------------- | --------------------------------------- |
| `je`  | `cmove`  | `ZF = 1`                    | `rB == rA`                            |
| `jne` | `cmovne` | `ZF = 0`                    | `rB != rA`                            |
| `jl`  | `cmovl`  | `SF ^ OF = 1`               | `rB < rA`                             |
| `jle` | `cmovle` | `(SF ^ OF) 或 ZF`           | `rB <= rA`                            |
| `jge` | `cmovge` | `SF ^ OF = 0`               | `rB >= rA`                            |
| `jg`  | `cmovg`  | `SF ^ OF = 0` 且 `ZF = 0` | `rB > rA`                             |

大小关系按**有符号数**解释。`cmovXX rA, rB` 在条件成立时复制寄存器值，否则保持目的寄存器不变；它不会跳转，也不修改条件码。

```asm
andq %rdi, %rdi       # 判断指针是否为 NULL
je done              # %rdi 为 0 时跳转

# 比较两个值时，先复制到临时寄存器，避免 subq 改掉原值
rrmovq %rdi, %r10
subq %rsi, %r10       # 临时计算 %rdi - %rsi
jg greater           # 原来的 %rdi > %rsi 时跳转
```

跳转只看当前条件码；比较与跳转之间若插入其他算术/逻辑运算，条件码会被覆盖。

## 5. 栈与函数调用

| 指令           | 含义                                                                          |
| -------------- | ----------------------------------------------------------------------------- |
| `pushq rA`   | `%rsp` 减 8，再把寄存器值存入栈顶                                           |
| `popq rA`    | 从栈顶读取到寄存器，栈顶通常前进 8 字节；目的为 `%rsp` 时最终以弹出的值为准 |
| `call label` | 把下一条指令的地址压栈，然后跳到 `label`                                    |
| `ret`        | 从栈顶弹出返回地址，并跳到该地址                                              |

这些指令不修改条件码。`call` 不会自动保存通用寄存器，函数需要按调用约定自行保存和恢复；递归时尤其注意保存后续仍要使用的值。

## 6. 程序控制

| 指令     | 含义                       |
| -------- | -------------------------- |
| `halt` | 停止程序执行               |
| `nop`  | 空操作，继续执行下一条指令 |

## 7. 汇编伪指令

伪指令由汇编器处理，用于安排代码和数据，不是 CPU 执行的指令。

| 写法                              | 含义                                     |
| --------------------------------- | ---------------------------------------- |
| `.pos 0`                        | 将后续代码或数据的起始地址设为 0         |
| `.align 8`                      | 将当前位置对齐到 8 的倍数                |
| `.quad 10`                      | 存放一个 8 字节整数 10，也可存放标签地址 |
| `.byte` / `.word` / `.long` | 分别存放 1 / 2 / 4 字节数据              |

例如链表节点的两个字段各占 8 字节：

```asm
.align 8
node1:
    .quad 10         # val，偏移 0
    .quad node2      # next，偏移 8
node2:
    .quad 20
    .quad 0          # NULL
```

## 8. 实验中的扩展与运行命令

`iaddq $8, %rdi` 表示将立即数 8 加到 `%rdi`，并更新条件码。它是实验涉及的扩展指令：`yas` 能汇编不代表待实现的 SEQ/PIPE 处理器已支持，做 Part A 时先使用基础指令。

在 `archlab-handout/sim/misc` 中：

```bash
./yas sum.ys          # 汇编，生成 sum.yo
./yis sum.yo          # 用指令集模拟器运行，查看寄存器和内存变化
```

## 9. 环境配置与构建问题

以下记录本次在 Ubuntu、GCC 15.2 环境中遇到的问题。

### 安装依赖并构建

安装词法分析器生成工具 `flex` 和语法分析器生成工具 `bison`：

```bash
sudo apt update
sudo apt install flex bison
```

在 `archlab-handout/sim` 目录构建全部工具：

```bash
make -B CC="gcc -std=gnu17 -fcommon"
```

若只需构建 Part A 使用的工具，也可以在 `archlab-handout/sim/misc` 中执行相同命令。

| 参数 | 作用 |
| --- | --- |
| `CC="gcc ..."` | 覆盖 Makefile 的 C 编译器变量，参数也会传给子目录的 make |
| `-std=gnu17` | 使用 GNU C17，兼容旧代码的函数声明写法 |
| `-fcommon` | 允许合并多个编译单元中同名的暂定全局变量定义，兼容旧代码 |
| `-B` | 强制重新构建，确保已有 `.o` 文件也使用新参数 |

这些参数只对本次构建生效，不会修改 Makefile 或系统默认设置。后续构建继续使用相同的 `CC` 参数；不需要强制重编译时可以省略 `-B`。

### 问题一：free_reg 参数过多

报错示例：

```text
error: too many arguments to function ‘free_reg’; expected 0, have 1
```

旧代码在 `isa.h` 中使用 `void free_reg();`。GCC 15 默认采用 GNU C23，该声明表示函数不接受参数，因此调用 `free_reg(saver)` 会报错；旧标准下，空参数列表表示未指定参数信息。

解决：使用 `-std=gnu17`。[GCC 15 迁移说明](https://gcc.gnu.org/gcc-15/porting_to.html)

### 问题二：找不到 flex

报错示例：

```text
make[1]: flex: No such file or directory
```

原因是缺少 `flex`，无法根据 `yas-grammar.lex` 生成 C 源码。安装上面的 `flex`、`bison` 后重新构建即可。

### 问题三：lineno 重复定义

报错示例：

```text
multiple definition of `lineno'
collect2: error: ld returned 1 exit status
```

`yas.h` 中的 `int lineno;` 被多个源文件包含，产生了重复定义。GCC 从 10 起默认使用 `-fno-common`，链接时会拒绝这种写法。

解决：加入 `-fcommon`，并使用 `-B` 重新编译已有目标文件；仅在链接时加入参数不足以修复旧的 `.o` 文件。[GCC 10 迁移说明](https://gcc.gnu.org/gcc-10/porting_to.html)

### 问题四：找不到 ./yas

报错示例：

```text
bash: ./yas: No such file or directory
```

本次是因为前面的构建失败，尚未生成 `yas`，与 `sum.ys` 的内容无关。在 `sim/misc` 中成功构建后执行：

```bash
./yas sum.ys
./yis sum.yo
```

注意：`./yas` 表示运行当前目录下的程序，因此上述命令需要在 `sim/misc` 中执行。

### make clean 的警告

```text
warning: ignoring prerequisites on suffix rule definition
```

这是旧 Makefile 的后缀规则写法引起的警告，不是前面几个构建失败的原因。`make clean` 用于删除构建产物；排查上述兼容问题时使用 `make -B` 即可强制重建，无需先清理。

## 10. Part A 实验结果

### 第一个程序：sum.ys（迭代求链表元素之和）

使用 PDF 给出的三个节点值 `0x00a`、`0x0b0`、`0xc00` 测试，预期结果为 `0xcba`。在 `archlab-handout/sim/misc` 中运行：

```text
stone@hate:~/projects/csapp-labs-stone/archlab-handout/sim/misc$ ./yas sum.ys
stone@hate:~/projects/csapp-labs-stone/archlab-handout/sim/misc$ ./yis sum.yo
Stopped in 26 steps at PC = 0x1d.  Status 'HLT', CC Z=1 S=0 O=0
Changes to registers:
%rax:   0x0000000000000000      0x0000000000000cba
%rsp:   0x0000000000000000      0x0000000000000200

Changes to memory:
0x01f8: 0x0000000000000000      0x000000000000001d
```

本次三节点链表测试通过：

- `yas` 没有报错，汇编成功。
- 状态为 `HLT`，程序执行到 `halt` 正常停止。
- `%rax = 0xcba`，与预期的链表元素之和一致。
- `%rsp = 0x200`，函数返回后栈顶恢复到初始位置。
- 地址 `0x1f8` 中的 `0x1d` 是 `call` 压入的返回地址；`ret` 只移动栈指针，不清除该内存，因此保留这个值是正常现象。

### 第二个程序：rsum.ys（递归求链表元素之和）

使用与 `sum.ys` 相同的三个节点值 `0x00a`、`0x0b0`、`0xc00` 测试，预期结果为 `0xcba`。运行记录如下：

```text
stone@hate:~/projects/csapp-labs-stone/archlab-handout/sim/misc$ ./yis rsum.yo
Stopped in 35 steps at PC = 0x1d.  Status 'HLT', CC Z=0 S=0 O=0
Changes to registers:
%rax:   0x0000000000000000      0x0000000000000cba
%rsp:   0x0000000000000000      0x0000000000000200
%r10:   0x0000000000000000      0x000000000000000a

Changes to memory:
0x01c8: 0x0000000000000000      0x0000000000000053
0x01d0: 0x0000000000000000      0x0000000000000c00
0x01d8: 0x0000000000000000      0x0000000000000053
0x01e0: 0x0000000000000000      0x00000000000000b0
0x01e8: 0x0000000000000000      0x0000000000000053
0x01f0: 0x0000000000000000      0x000000000000000a
0x01f8: 0x0000000000000000      0x000000000000001d
```

递归求和测试通过：

- 状态为 `HLT`，程序正常停止。
- `%rax = 0xcba`，与三个节点值之和一致。
- `%rsp` 最终恢复到 `0x200`，递归过程中的栈空间已逐层弹回。
- `0x1f0`、`0x1e0`、`0x1d0` 分别留有递归过程中保存的节点值；`0x1f8` 留有 `call` 保存的返回地址。这些栈内存不会因 `popq` 或 `ret` 自动清零。
