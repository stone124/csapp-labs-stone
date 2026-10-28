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

| 参数             | 作用                                                     |
| ---------------- | -------------------------------------------------------- |
| `CC="gcc ..."` | 覆盖 Makefile 的 C 编译器变量，参数也会传给子目录的 make |
| `-std=gnu17`   | 使用 GNU C17，兼容旧代码的函数声明写法                   |
| `-fcommon`     | 允许合并多个编译单元中同名的暂定全局变量定义，兼容旧代码 |
| `-B`           | 强制重新构建，确保已有 `.o` 文件也使用新参数           |

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

### 第三个程序：copy.ys（复制数据块并计算异或校验值）

使用源数据 `0x00a`、`0x0b0`、`0xc00`，目标数据初始为 `0x111`、`0x222`、`0x333`，长度为 3。预期复制后目标数据与源数据一致，返回值为 `0x00a ^ 0x0b0 ^ 0xc00 = 0xcba`。本次运行记录如下：

```text
stone@hate:~/projects/csapp-labs-stone/archlab-handout/sim/misc$ ./yas copy.ys
stone@hate:~/projects/csapp-labs-stone/archlab-handout/sim/misc$ ./yis copy.yo
Stopped in 38 steps at PC = 0x31.  Status 'HLT', CC Z=1 S=0 O=0
Changes to registers:
%rax:   0x0000000000000000      0x0000000000000cba
%rsp:   0x0000000000000000      0x0000000000000200
%rsi:   0x0000000000000000      0x00000000000000c0
%rdi:   0x0000000000000000      0x00000000000000a8
%r8:    0x0000000000000000      0x0000000000000c00
%r9:    0x0000000000000000      0x0000000000000008
%r11:   0x0000000000000000      0x0000000000000001

Changes to memory:
0x00a8: 0x0000000000000111      0x000000000000000a
0x00b0: 0x0000000000000222      0x00000000000000b0
0x00b8: 0x0000000000000333      0x0000000000000c00
0x01f8: 0x0000000000000000      0x0000000000000031
```

本次三元素数据块测试通过：

- 状态为 `HLT`，程序正常停止。
- `%rax = 0xcba`，与源数据的异或校验值一致。
- `dest` 的三个地址 `0xa8`、`0xb0`、`0xb8` 分别写入 `0xa`、`0xb0`、`0xc00`，复制结果正确。
- `%rdi` 和 `%rsi` 分别指向源数据块和目标数据块末尾之后的位置；`%rdx` 最终为 0，与运行前相同，因此未列在寄存器变化中。
- `%rsp` 恢复到 `0x200`；`0x1f8` 中的 `0x31` 是调用时保存的返回地址，函数返回后该内存仍保留原值。

## 11. Part B：让 SEQ 支持 iaddq

### 要做什么

修改 `sim/seq/seq-full.hcl`，让 SEQ 处理器支持 `iaddq V, rB`：把立即数 `V` 加到寄存器 `rB`，结果写回 `rB`，同时更新条件码。例如 `iaddq $8, %rdi` 相当于给 `%rdi` 加 8。

根据这条指令需要的数据，在对应控制信号中加入 `IIADDQ`：

| 位置       | 修改内容                                                                                               |
| ---------- | ------------------------------------------------------------------------------------------------------ |
| 取指       | 在 `instr_valid`、`need_regids`、`need_valC` 中加入 `IIADDQ`，识别指令并读取寄存器编号和立即数 |
| 译码、写回 | `srcB` 和 `dstE` 选择 `rB`，读取原值并将运算结果写回                                             |
| 执行       | `aluA` 选择 `valC`，`aluB` 选择 `valB`，计算立即数加寄存器值                                   |
| 条件码     | 在 `set_cc` 中加入 `IIADDQ`，更新 ZF、SF、OF                                                       |

`alufun` 默认就是 `ALUADD`，可以直接使用；这条指令不读写数据内存，也不需要额外修改跳转逻辑。

### 构建和验证

在 `archlab-handout/sim/seq` 中构建命令行版本，并运行包含 `iaddq` 的示例：

```bash
make VERSION=full CFLAGS="-Wall -O2 -std=gnu89 -fcommon" GUIMODE= TKLIBS= TKINC=
../misc/yas ../y86-code/asumi.ys
./ssim -t ../y86-code/asumi.yo
```

空的 `GUIMODE`、`TKLIBS`、`TKINC` 用于关闭 GUI 构建，避免依赖 Tcl/Tk。`-t` 会把 SEQ 的执行结果与 ISA 模拟器进行比较。本次输出节选：

```text
32 instructions executed
Status = HLT
Condition Codes: Z=1 S=0 O=0
Changed Register State:
%rax:   0x0000000000000000      0x0000abcdabcdabcd
%rsp:   0x0000000000000000      0x0000000000000100
%rdi:   0x0000000000000000      0x0000000000000038
%r10:   0x0000000000000000      0x0000a000a000a000
Changed Memory State:
0x00f0: 0x0000000000000000      0x0000000000000055
0x00f8: 0x0000000000000000      0x0000000000000013
ISA Check Succeeds
```

接着运行原有程序的回归测试，以及包含 `iaddq` 的测试：

```bash
cd ../y86-code
make testssim
cd ../ptest
make SIM=../seq/ssim TFLAGS=-i
```

本次 `make testssim` 的 16 个示例均输出 `ISA Check Succeeds`；`ptest` 输出如下：

```text
./optest.pl -s ../seq/ssim -i
Simulating with ../seq/ssim
  All 58 ISA Checks Succeed
./jtest.pl -s ../seq/ssim -i
Simulating with ../seq/ssim
  All 96 ISA Checks Succeed
./ctest.pl -s ../seq/ssim -i
Simulating with ../seq/ssim
  All 22 ISA Checks Succeed
./htest.pl -s ../seq/ssim -i
Simulating with ../seq/ssim
  All 756 ISA Checks Succeed
```

以上运行结果表明：`asumi.ys` 执行正确，原有示例未出现回归，带 `-i` 的四组测试共 932 项 ISA 检查全部通过。

## 12. Part C：优化 ncopy 与 PIPE

### 12.1 要做什么，以及怎样判断优化有效

Part C 同时优化两个文件：

| 文件 | 作用 | 本次采用的修改 |
| --- | --- | --- |
| [`sim/pipe/ncopy.ys`](sim/pipe/ncopy.ys) | 复制数组，并返回正数元素的个数 | 减少重复指令、八路展开、预扣长度、按余数跳入尾部标签、调整判断顺序 |
| [`sim/pipe/pipe-full.hcl`](sim/pipe/pipe-full.hcl) | 定义 PIPE 的控制逻辑 | 支持 `iaddq`、加载到存储的数据转发、BTFNT 分支预测 |

汇编不必逐行对应 C 的写法，但要保持相同的功能：复制 `len` 个元素到目标数组，统计其中严格大于 0 的元素。这里统计的是正数个数，不是 Part A 的异或校验值；0 不计入正数。

```c
long ncopy(long *src, long *dst, long len)
{
    long count = 0;
    while (len > 0) {
        long val = *src++;
        *dst++ = val;
        if (val > 0)
            count++;
        len--;
    }
    return count;
}
```

优化可以从两个大方向入手：

1. **减少实际执行的指令。** 例如把循环不变量移到循环外，删除重复判断，用 `iaddq` 合并常数加载和加法，用循环展开分摊指针更新与循环判断。
2. **减少流水线损失。** 一方面让独立指令隔开暂时拿不到结果的数据依赖，或增加转发路径；另一方面减少分支预测失败的次数，或改进预测失败后的恢复过程。

原先把目标概括成“打满流水线”，可以作为直观理解，但还要继续看每个元素执行了多少条指令。即使没有气泡，多执行十条不必要的指令也仍然要花时间。有寄存器依赖也不一定会停顿：如果现有转发能及时提供结果，后续指令就可以正常前进。

“让分支更容易预测”和“减少每次预测失败的代价”也要区分。本次 BTFNT 主要改变预测方向和失败次数；没有把条件判断提前到 D 阶段，分支仍在 E 判断、通过 M 阶段保存的信息恢复取指。

每轮修改先检查正确性，再比较性能。在 `archlab-handout/sim/pipe` 执行：

```bash
# 修改 HCL 后重新构建；仅修改 ncopy.ys 时通常无需重建 psim
make -B psim VERSION=full CFLAGS="-Wall -O2 -std=gnu89 -fcommon" GUIMODE= TKLIBS= TKINC=

# 检查汇编本身的功能
./correctness.pl -f ncopy.ys

# 检查同一份汇编在当前 PIPE 上的功能
./correctness.pl -p -f ncopy.ys

# 两项均通过后比较性能
./benchmark.pl -f ncopy.ys
```

`GUIMODE=`、`TKLIBS=`、`TKINC=` 用于构建命令行版本，避免 GUI 的 Tcl/Tk 依赖。`-B` 强制重新生成和构建模拟器，适合修改 HCL 后使用。`correctness.pl` 可以增加 `-q` 只显示汇总；不加 `-q` 的命令保留了每项测试的详细输出。

`correctness.pl` 默认对 0～64，以及 128、192、256 个元素进行测试，共 68 项，并检查复制结果、正数计数和默认 1000 字节的代码长度限制。YIS 检查通过说明汇编在指令集模型上正确；PIPE 检查通过说明当前流水线实现也能正确执行它。修改 HCL 后还需要跑后面记录的通用 ISA 回归测试，不能只验证 `ncopy`。

### 12.2 第一轮：隔开加载依赖，调整循环入口

最明显的数据依赖来自相邻的加载和存储：

```asm
Loop:
    mrmovq (%rdi), %r10    # 从 src 加载当前元素
    rmmovq %r10, (%rsi)    # 立即使用刚加载的元素
```

第二条指令需要第一条从内存读出的值。标准 PIPE 中，加载结果要到 M 阶段才能产生；直接相邻的使用者会遇到 load/use 冒险。

这里需要纠正原先笔记里的一个理解：**不必等加载指令的 W 阶段结束后，后一条才能使用结果。** 标准 PIPE 已有 M 到 D 的转发，但相邻使用者原本在加载处于 E 时就进入 D，时间仍然早了一拍，所以需要等待一拍。

一种汇编层面的处理方法，是把独立的指针更新插进来。当后续指令真正需要 `%r10` 时，加载结果已经可用：

```asm
Loop:
    mrmovq (%rdi), %r10
    irmovq $8, %r11
    addq %r11, %rdi       # src++
    addq %r11, %rsi       # dst++
    rmmovq %r10, -8(%rsi)
```

这里保留了当时的尝试写法。一个元素为 8 字节，`%rsi` 已经加了 8，所以原来的目标位置要写成 `-8(%rsi)`。原来想“把常数提前存到 `%r11`”，进一步就应该把 `irmovq $8, %r11` 移到循环外；上面的中间版本只是把它放在了加载与存储之间，循环中仍有一次重复赋值。

同时考虑循环入口的条件方向。标准 PIPE 对条件跳转采用“总是预测跳转”，所以可以根据实际常走的方向组织跳转。例如入口若大多数是 `len > 0`，可以写成：

```asm
    andq %rdx, %rdx       # 根据 len 设置条件码
    jg Loop              # len > 0 时进入循环
    ret                  # len <= 0 时返回
```

需要注意，原来注释写 `len <= 0?`，后来改成 `len > 0?`，**单改注释不会改变预测或功能**。真正改变控制流的是 `jg` / `jle` 的选择、跳转目标和代码排列。`andq %rdx, %rdx` 只负责设置条件码，本身不决定是在判断大于还是小于。

第一版修改后，按上一节的构建、正确性、性能命令运行，记录为：

```text
Average CPE     14.03
Score   0.0/60.0
```

### 12.3 第二轮：提取公共指令，删除重复判断

继续检查有没有“不管哪个分支都会做”的工作。当时的计数和长度更新附近有：

```asm
    jle Npos
    irmovq $1, %r10
    addq %r10, %rax       # count++
Npos:
    irmovq $1, %r10
```

正数路径会先加载一次常数 1，经过 `Npos` 又加载一次；非正数路径也要加载常数 1 才能做 `len--`。可以把公共的常数加载提前，或更进一步，在循环外使用专门的常数寄存器。

提前时要检查寄存器的最后一次用途：如果 `%r10` 还保存着待存储的元素，就不能提前用常数 1 覆盖它；应当先完成存储和对元素的 `andq`。`irmovq` 不修改条件码，所以在已经做完 `andq` 后加载常数，不会改变紧接着的 `jle` 判断。

另一个重复操作是：

```asm
    subq %r10, %rdx       # len--，同时设置条件码
    andq %rdx, %rdx       # 再次根据减完后的 len 设置条件码
    jg Loop
```

正常循环路径中，已知 `len > 0` 且 `%r10 = 1`，`len - 1` 不发生有符号溢出。`subq` 已经提供了判断新长度是否大于 0 所需的条件码，因此可以删除中间的 `andq`：

```asm
    subq %r10, %rdx
    jg Loop
```

这并不表示算术操作与 `andq` 在所有输入下的全部条件码都完全相同。`OF` 反映算术过程是否溢出；这里能删除，是因为具体循环范围保证减 1 不溢出，并且中间没有其他修改条件码的指令。

精简后的记录：

```text
Average CPE     12.55
Score   0.0/60.0
```

比第一版好了，但仍未达到得分范围。下一步需要减少每个元素分摊的循环控制开销。

### 12.4 第三轮：循环展开，并把常数移出循环

程序优化一章介绍了循环展开：原来一轮处理一个元素，现在一轮处理多个元素。原先的想法是“拆成两行并行”，更准确地说，是暴露多个元素之间可以独立安排的工作，并把指针更新、长度更新、循环判断分摊到多个元素上。PIPE 仍然不是同一周期发射多条指令的处理器。

二路展开时，要同时处理三个问题：

1. 主循环入口必须确认至少有两个元素，即 `len > 1`。
2. 两个元素都要复制，并且各自判断是否为正，不能用其中一个元素的条件码代替另一个。
3. 每轮消耗两个元素，退出后可能剩 0 或 1 个，需要单独的 `Remain` 处理。

当时的二路展开代码保留如下，便于对照后来的改进：

```asm
	# Loop header
	xorq %rax,%rax		# count = 0;
	rrmovq %rdx, %r8
	irmovq $1, %r10
	subq %r10, %r8		# len > 1?
	jg Loop		# if so, goto Loop:
	jmp Remain # if not, remain

Loop:
	mrmovq (%rdi), %r10	# read val from src...
	mrmovq 8(%rdi), %r8
	irmovq $16, %r11
	addq %r11, %rdi		# src+2
	addq %r11, %rsi		# dst+2
	rmmovq %r10, -16(%rsi)	# ...and store it to dst
	rmmovq %r8, -8(%rsi)
	andq %r10, %r10		# val1 <= 0?
	irmovq $1, %r10
	jle Val
	addq %r10, %rax
Val:
	andq %r8, %r8  # val2 <= 0
	jle Npos		# if so, goto Npos:
	addq %r10, %rax		# count++
Npos:
	irmovq $2, %r10
	subq %r10, %rdx		# len-2
	rrmovq %rdx, %r8
	irmovq $1, %r10
	subq %r10, %r8		# len > 1?
	jg Loop			# if so, goto Loop:
Remain:
	andq %rdx, %rdx # len <= 0
	jle Done
	mrmovq (%rdi), %r10 # src[0]读出val
	rmmovq %r10, (%rsi) # 放到dst里面

	andq %r10, %r10
	jle Done # val <=0, jump done
	irmovq $1, %r10
	addq %r10, %rax
```

这里比较 `len > 1` 的方法，是先把长度复制到 `%r8`，再减 1，最后用 `jg` 判断；这样暂时不改变实际长度 `%rdx`。指针每轮前进 16 字节，存储位置相应使用 `-16(%rsi)` 和 `-8(%rsi)`。

`andq %r10, %r10` 后的 `irmovq $1, %r10` 不修改条件码，因此 `jle Val` 仍然是在判断第一个元素。到第二个元素时，要重新执行 `andq %r8, %r8`。

二路展开后的结果：

```text
Average CPE     11.78
Score   0.0/60.0
```

这版循环内部仍有很多常数赋值：1 用于计数，2 用于长度更新，16 用于指针更新。它们不会随迭代变化，可以统一移到前面：

```asm
    irmovq $1, %r9        # 常数 1，用于 count++
    irmovq $2, %rcx       # 常数 2，用于每轮消耗两个元素
    irmovq $16, %r11      # 常数 16，用于指针前进两个元素
```

后续循环只使用这些寄存器，不再反复 `irmovq`，结果为：

```text
Average CPE     10.01
Score   9.8/60.0
```

继续增加展开数量，原始记录如下：

| 当时尝试的展开数量 | Average CPE | Score |
| --- | ---: | ---: |
| 四路展开 | 9.10 | 28.0/60.0 |
| 八路展开 | 8.83 | 33.4/60.0 |
| 十六路展开 | 9.17 | 26.6/60.0 |

在这组对比中，八路最好，所以保留八路。展开越多不一定越好：主循环开销更少，但代码变长、入口和尾部处理更复杂，小数组可能还没有进入大主循环就已经结束。后来还试过十路、十一路，结果见后面的补充尝试，不能把“八路最好”理解成所有实现与所有条件下都成立。

### 12.5 第四轮：修改 HCL，增加加载到存储的转发

#### 两条指令到底分别需要什么

汇编层面分析过后，继续看 `pipe-full.hcl`。目标仍是这组相邻指令：

```asm
mrmovq (%rdi), %r10      # 指令 1：从内存加载到 %r10
rmmovq %r10, (%rsi)      # 指令 2：把 %r10 存到内存
```

`rmmovq rA, D(rB)` 有两种不同用途的数据：

| 操作数 | 在译码中对应 | 用途 | 最迟什么时候需要 |
| --- | --- | --- | --- |
| `rA`，例如 `%r10` | `d_srcA` | 准备写入内存的值 | M 阶段真正写内存时 |
| `rB`，例如 `%rsi` | `d_srcB` | 目标地址的基址 | E 阶段计算 `D + rB` 时 |

寄存器编号和寄存器里的数值是两回事。例如 `E_dstM == d_srcA` 比较的是“加载要写哪个寄存器”和“存储的数据来自哪个寄存器”，不是比较两份元素数值是否相等。

标准流水线对 load/use 的处理是冻结 F、D，并在 E 插入气泡：

```text
周期                         1    2    3         4         5    6    7
mrmovq                       F    D    E         M         W
rmmovq                            F    D(等待)   D(转发)   E    M    W
E 中插入的气泡                                  bubble
```

加载指令不会卡在 E，它继续进入 M。被保留在 D 的是后面的使用者；气泡为加载结果的产生留出一拍。

#### 新增 M 到 E 的待存数据转发

存储的数据在 E 阶段还不参与地址计算，可以先携带旧值进入 E，再在 E 用前面加载指令当前 M 阶段读出的正确值替换它。下一拍正确的数据随存储指令进入 M，供写内存使用。

```hcl
word e_valA = [
    E_icode == IRMMOVQ &&
    M_icode in { IMRMOVQ, IPOPQ } &&
    E_srcA == M_dstM : m_valM;
    1 : E_valA;
];
```

这段选择逻辑按顺序检查，第一条条件成立就选 `m_valM`，否则选原来的 `E_valA`。几个名字分别表示：

| 名字 | 含义 |
| --- | --- |
| `E_icode` | 当前 E 流水线寄存器中指令的操作码；这里检查是否为存储 |
| `M_icode` | 当前 M 流水线寄存器中指令的操作码；这里检查是否为加载或弹栈 |
| `E_srcA` | E 中存储指令的数据源寄存器编号 |
| `M_dstM` | M 中加载指令从内存读取后要写回的寄存器编号 |
| `m_valM` | 当前周期 M 阶段刚从数据内存读出的值 |
| `E_valA` | E 流水线寄存器原来携带的 A 值 |
| `e_valA` | 当前周期 E 阶段最终送往下一流水线寄存器的 A 值 |

大写前缀 `E_`、`M_` 表示阶段入口流水线寄存器中已经保存的信息；小写前缀 `e_`、`m_` 表示该阶段当前周期组合逻辑产生的信号。因此 `M_dstM` 是寄存器编号，`m_valM` 才是要转发的内存数据，不能拿 `M_dstE` 代替前者。

取消这类暂停后的时间安排是：

```text
周期                         1    2    3    4                         5    6
mrmovq                       F    D    E    M(产生 m_valM)            W
rmmovq                            F    D    E(选 m_valM 为 e_valA)    M    W
```

这不是给 M 阶段新增一次读通用寄存器的动作，而是在数据到达时增加一条转发选择路径。

#### 只取消“待存数据依赖”的暂停

有了转发之后，还要修改 `Pipeline Register Control`。原来的加载冒险条件是：

```hcl
E_icode in { IMRMOVQ, IPOPQ } &&
E_dstM in { d_srcA, d_srcB }
```

这里说“E 中有加载指令”，不是说 E 已经读出内存数据：此时加载在 E 计算地址，下一拍进入 M 才读取内存。

可以排除的情况是：D 中是 `rmmovq`，加载结果只用于它的待存数据，并不用于地址基址：

```hcl
D_icode == IRMMOVQ &&
E_dstM == d_srcA &&
E_dstM != d_srcB
```

原来条件表示“需要等待”；上面条件表示“现在有转发，可以不等”。所以新条件是“原来的等待条件，并且不是这个例外”：

```hcl
E_icode in { IMRMOVQ, IPOPQ } &&
E_dstM in { d_srcA, d_srcB } &&
!(D_icode == IRMMOVQ &&
  E_dstM == d_srcA &&
  E_dstM != d_srcB)
```

把这项加载冒险判断同时用于 `F_stall`、`D_stall`、`E_bubble`。不能只去掉 `D_stall` 或 `E_bubble` 中的一处，否则前后阶段的控制会不配套。原有 `ret` 等暂停、分支错误清除、异常处理仍要保留。

`stall` 冻结的是相应流水线寄存器整组字段，`bubble` 则把下一拍该寄存器中的内容换成不产生正常指令副作用的气泡，不是随意填入可能被执行的假数值。某个寄存器 stall 不会自动让所有更前面的寄存器一起冻结，所以 F、D 的冻结要分别控制。

如果依赖的是目标地址，仍然需要等：

```asm
mrmovq (%rdi), %rsi
rmmovq %r10, (%rsi)     # 依赖 d_srcB，E 阶段就要用它算地址
```

因此不能直接把 `E_dstM in { d_srcA, d_srcB }` 简化成只检查 `d_srcB`：其他指令，例如 `addq`，仍然可能需要 `d_srcA` 对应的加载结果及时到达 E。也不能对全部 `rmmovq` 无条件取消暂停。如果同一个寄存器既是待存数据又是地址基址，`E_dstM != d_srcB` 会阻止错误地放行。

#### 当时的通用回归输出

修改后，在 `y86-code` 跑示例，再到 `ptest` 跑基础 ISA 回归。当时的完整终端记录保留如下；这些命令对应当时已生成好 `.yo` 示例文件的工作目录：

```text
stone@hate:~/projects/csapp-labs-stone/archlab-handout/sim/pipe$ cd ../y86-code
make testpsim
Makefile:42: warning: ignoring prerequisites on suffix rule definition
Makefile:45: warning: ignoring prerequisites on suffix rule definition
Makefile:48: warning: ignoring prerequisites on suffix rule definition
Makefile:51: warning: ignoring prerequisites on suffix rule definition
../pipe/psim -t asum.yo > asum.pipe
../pipe/psim -t asumr.yo > asumr.pipe
../pipe/psim -t cjr.yo > cjr.pipe
../pipe/psim -t j-cc.yo > j-cc.pipe
../pipe/psim -t poptest.yo > poptest.pipe
../pipe/psim -t pushquestion.yo > pushquestion.pipe
../pipe/psim -t pushtest.yo > pushtest.pipe
../pipe/psim -t prog1.yo > prog1.pipe
../pipe/psim -t prog2.yo > prog2.pipe
../pipe/psim -t prog3.yo > prog3.pipe
../pipe/psim -t prog4.yo > prog4.pipe
../pipe/psim -t prog5.yo > prog5.pipe
../pipe/psim -t prog6.yo > prog6.pipe
../pipe/psim -t prog7.yo > prog7.pipe
../pipe/psim -t prog8.yo > prog8.pipe
../pipe/psim -t ret-hazard.yo > ret-hazard.pipe
grep "ISA Check" *.pipe
asum.pipe:ISA Check Succeeds
asumr.pipe:ISA Check Succeeds
cjr.pipe:ISA Check Succeeds
j-cc.pipe:ISA Check Succeeds
poptest.pipe:ISA Check Succeeds
prog1.pipe:ISA Check Succeeds
prog2.pipe:ISA Check Succeeds
prog3.pipe:ISA Check Succeeds
prog4.pipe:ISA Check Succeeds
prog5.pipe:ISA Check Succeeds
prog6.pipe:ISA Check Succeeds
prog7.pipe:ISA Check Succeeds
prog8.pipe:ISA Check Succeeds
pushquestion.pipe:ISA Check Succeeds
pushtest.pipe:ISA Check Succeeds
ret-hazard.pipe:ISA Check Succeeds
rm asum.pipe asumr.pipe cjr.pipe j-cc.pipe poptest.pipe pushquestion.pipe pushtest.pipe prog1.pipe prog2.pipe prog3.pipe prog4.pipe prog5.pipe prog6.pipe prog7.pipe prog8.pipe ret-hazard.pipe
stone@hate:~/projects/csapp-labs-stone/archlab-handout/sim/y86-code$ cd ../ptest
make SIM=../pipe/psim
./optest.pl -s ../pipe/psim
Simulating with ../pipe/psim
  All 49 ISA Checks Succeed
./jtest.pl -s ../pipe/psim
Simulating with ../pipe/psim
  All 64 ISA Checks Succeed
./ctest.pl -s ../pipe/psim
Simulating with ../pipe/psim
  All 22 ISA Checks Succeed
./htest.pl -s ../pipe/psim
Simulating with ../pipe/psim
  All 600 ISA Checks Succeed
stone@hate:~/projects/csapp-labs-stone/archlab-handout/sim/ptest$
```

16 个示例均输出 `ISA Check Succeeds`，四组基础测试共 `49 + 64 + 22 + 600 = 735` 项检查通过。Makefile 中 suffix rule 的警告没有使这些检查失败；它与之前环境配置一节记录的警告相同。

随后回到 `sim/pipe` 检查 `ncopy` 和性能：

```bash
./correctness.pl -p -f ncopy.ys
./benchmark.pl -f ncopy.ys
```

当时原始笔记记录为：

```text
Average CPE     8.83
Score   33.4/60.0
```

数值没有变化，不代表转发无效，而是当时已经通过指令重排和展开，把很多加载后立即使用的情形避开了。硬件只在实际经过新路径、并且原来确实会暂停的位置才省周期。后面尾部重新采用相邻的加载、存储时，这条转发路径仍然有用。

### 12.6 从完整性能表定位问题：短数组和尾部开销

当时 `benchmark.pl` 的完整输出如下，保留所有长度，便于观察每 8 个元素的变化：

```text
ncopy
0       22
1       35      35.00
2       46      23.00
3       54      18.00
4       65      16.25
5       73      14.60
6       84      14.00
7       92      13.14
8       71      8.88
9       84      9.33
10      95      9.50
11      103     9.36
12      114     9.50
13      122     9.38
14      133     9.50
15      141     9.40
16      121     7.56
17      134     7.88
18      145     8.06
19      153     8.05
20      164     8.20
21      172     8.19
22      183     8.32
23      191     8.30
24      171     7.12
25      184     7.36
26      195     7.50
27      203     7.52
28      214     7.64
29      222     7.66
30      233     7.77
31      241     7.77
32      221     6.91
33      234     7.09
34      245     7.21
35      253     7.23
36      264     7.33
37      272     7.35
38      283     7.45
39      291     7.46
40      271     6.78
41      284     6.93
42      295     7.02
43      303     7.05
44      314     7.14
45      322     7.16
46      333     7.24
47      341     7.26
48      321     6.69
49      334     6.82
50      345     6.90
51      353     6.92
52      364     7.00
53      372     7.02
54      383     7.09
55      391     7.11
56      371     6.62
57      384     6.74
58      395     6.81
59      403     6.83
60      414     6.90
61      422     6.92
62      433     6.98
63      441     7.00
64      421     6.58
Average CPE     8.83
Score   33.4/60.0
```

三列分别是：元素个数 `n`、模拟器报告的周期数 `cycles(n)`、该长度的 `CPE(n) = cycles(n) / n`。例如 `1 35 35.00` 表示 1 个元素用了 35 个周期；`8 71 8.88` 表示 8 个元素用了 71 个周期。

`n = 0` 没有可除的元素个数，所以只打印周期数。根据本仓库的 `benchmark.pl`，默认平均值为：

```text
Average CPE = (CPE(1) + CPE(2) + ... + CPE(64)) / 64
```

它不是把所有周期求和后除以所有元素总数。`n = 0` 虽然打印，但不参与默认平均 CPE。

这解释了为什么 0～15 的表现值得特别关注，尤其是 1～7：入口初始化与尾部控制的固定成本会被很小的 `n` 放大。同样省 1 个周期，`n = 1` 对平均值的影响是 `n = 64` 的 64 倍。8 的倍数附近 CPE 明显下降，则说明展开主循环效率较高、余数处理相对昂贵。

曾尝试让开头 0～7 个元素先走逐个处理路径，原笔记只记录“提升很小”，没有留下该版本精确结果。这个方向的限制是：即使短数组少走几步，如果所有长数组因此多一次入口判断或跳转，整体平均值也未必改善。需要比较所有长度，不能只看最短几项或最大长度。

本地评分脚本的非零分阈值是 10.5 CPE，满分阈值是 7.5 CPE，中间按线性方式给 60 分。这里记录的是本仓库脚本的结果；评分使用未四舍五入的平均值，所以不能总用终端显示的两位小数完全还原分数。

### 12.7 第五轮：在 PIPE 中支持并使用 iaddq

Part B 已在 `seq-full.hcl` 中实现 `iaddq`，但那只影响 SEQ；Part C 的 PIPE 使用另一份 `pipe-full.hcl`，不会自动继承。`yas` 能识别并汇编 `iaddq`，也不等于 PIPE 已经能执行它。

例如原来要两条指令：

```asm
irmovq $8, %r11
addq %r11, %rdi
```

支持 `iaddq` 后可以写成：

```asm
iaddq $8, %rdi
```

如果常数已经提到循环外，原本循环中的 `addq` 也是一条指令，所以不能把所有替换都算成“每次省一条”。这时主要还会省掉入口常数初始化、常数寄存器占用，并为后面直接加减比较提供方便。

PIPE 中需要把 `IIADDQ` 接入这些控制信号：

| 控制信号 | `iaddq V, rB` 所需行为 |
| --- | --- |
| `instr_valid` | 把 `IIADDQ` 视为合法指令 |
| `need_regids` | 读取寄存器编号字节 |
| `need_valC` | 读取立即数 V |
| `d_srcB` | 选择 `D_rB`，获取原寄存器值 |
| `d_dstE` | 选择 `D_rB`，把 ALU 结果写回同一个寄存器 |
| `aluA` | 选择 `E_valC`，即立即数 |
| `aluB` | 选择 `E_valB`，即寄存器原值 |
| `alufun` | 默认 `ALUADD` 即可 |
| `set_cc` | 将 `IIADDQ` 加入更新条件码的指令集合，保留异常状态限制 |

`d_srcA` 不需要为 `iaddq` 选择通用寄存器；这条指令的寄存器输入来自 B。已有数据转发和 `d_srcB` 加载冒险检测也会随这个选择工作，不需要为了 `iaddq` 删去原来的保护。

在 `sim/pipe` 重建，然后运行带 `iaddq` 的回归：

```bash
make -B psim VERSION=full CFLAGS="-Wall -O2 -std=gnu89 -fcommon" GUIMODE= TKLIBS= TKINC=
cd ../ptest
make SIM=../pipe/psim TFLAGS=-i
```

当时输出：

```text
stone@hate:~/projects/csapp-labs-stone/archlab-handout/sim/ptest$ make SIM=../pipe/psim TFLAGS=-i
./optest.pl -s ../pipe/psim -i
Simulating with ../pipe/psim
  All 58 ISA Checks Succeed
./jtest.pl -s ../pipe/psim -i
Simulating with ../pipe/psim
  All 96 ISA Checks Succeed
./ctest.pl -s ../pipe/psim -i
Simulating with ../pipe/psim
  All 22 ISA Checks Succeed
./htest.pl -s ../pipe/psim -i
Simulating with ../pipe/psim
  All 756 ISA Checks Succeed
```

四组共 `58 + 96 + 22 + 756 = 932` 项 ISA 检查通过。回到 `sim/pipe` 运行正确性和性能测试后：

```text
Average CPE     8.54
Score   39.2/60.0
```

### 12.8 第六轮：减少尾部的重复循环控制

#### 先按 4、2、1 分层

八路主循环退出后剩 0～7 个元素。逐个尾部循环每处理一个元素，都要更新源指针、目标指针、长度并重新跳转。可以先尝试按 4、2、1 分层：

```text
剩余至少 4 个：处理 4 个，余数减 4
剩余至少 2 个：处理 2 个，余数减 2
还剩 1 个：处理最后 1 个
```

例如剩 7 个按 `4 + 2 + 1` 处理，剩 6 个按 `4 + 2` 处理。它减少了反复逐元素控制的开销，结果是：

```text
Average CPE     8.47
Score   40.5/60.0
```

#### 让长度更新同时完成下一组判断：预扣长度

当时主循环末尾的逻辑是：

```asm
iaddq $-8, %rdx          # 实际剩余长度减 8
rrmovq %rdx, %r8         # 再复制剩余长度
iaddq $-8, %r8           # 用副本判断还够不够下一组
jge Loop
```

这里做了两类工作：先更新真实剩余长度，再复制它用于比较。可以改变 `%rdx` 的含义，让它始终保存“预扣下一组 8 个后的长度”，这样一次减法既更新状态，也提供下一次循环条件。

先看 `len = 19` 时，原来的逻辑：

```text
入口判断 19 >= 8 → 处理 8 个
剩 11，判断 11 >= 8 → 再处理 8 个
剩 3，判断 3 >= 8 → 不处理，进入尾部
```

改成预扣后：

```asm
    iaddq $-8, %rdx      # 19 → 11，预扣第一组
    jl Remain           # 原长度不足 8 才进入尾部

Loop:
    # 处理 8 个元素
    iaddq $-8, %rdx      # 第一轮：11 → 3；第二轮：3 → -5
    jge Loop            # 第一轮继续，第二轮退出

Remain:
    iaddq $8, %rdx       # -5 → 3，恢复真正剩余数量
    # 处理尾部
```

第一轮结束时 `%rdx = 3` 并不是“只剩 3 个没处理”，而是已经把下一组的 8 个也预扣掉了，实际还有 11 个。第二轮结束时 `%rdx = -5`，实际才剩 3 个。必须把这个寄存器含义贯穿入口、循环末尾和尾部，否则容易多处理或少处理一组。

这样的结果为：

```text
Average CPE     8.22
Score   45.5/60.0
```

当前版本又进一步改变了尾部比较基准，不再在尾部先恢复真实余数；主循环的预扣逻辑仍保留，后文会具体说明。

#### 类似 switch：定位数量后连续处理

参考其他实现后，改成给每个余数设置一个入口标签 `Tail1`～`Tail7`。先确定余数，跳到对应标签，之后连续向下执行：

```text
余数 7 → Tail7 → Tail6 → Tail5 → Tail4 → Tail3 → Tail2 → Tail1 → Done
余数 3 →                                 Tail3 → Tail2 → Tail1 → Done
余数 1 →                                                 Tail1 → Done
```

每个标签只处理一个固定位置：`Tail7` 处理偏移 48，`Tail6` 处理偏移 40，直到 `Tail1` 处理偏移 0。它是从当前剩余块的最后一个元素往前复制，不是把结果倒着存；源偏移与目标偏移相同，复制后的元素顺序不变。

示例：

```asm
Tail3:
    mrmovq 16(%rdi), %r10
    rmmovq %r10, 16(%rsi)
    andq %r10, %r10
    jle Tail2
    iaddq $1, %rax
Tail2:
    # 同样处理偏移 8，然后继续到 Tail1
Tail1:
    # 处理偏移 0，然后返回
```

这类似 `switch` 的标签定位加连续执行，但不需要跳转表。标签间既可以由 `jle` 跳过计数，也可以在计数后顺序落入下一个标签。定位完成后不再反复更新指针、长度或检查是否还有元素。

这种逆序尾部基于本题的数组复制使用场景；如果另一个题目允许源、目标按任意方式重叠，就要重新确认复制方向是否仍满足所需语义。

切换后的结果：

```text
Average CPE     7.81
Score   53.8/60.0
```

随后发现余数恢复的 `iaddq` 已经设置了条件码，不必再执行一次 `andq`：

```asm
iaddq $8, %rdx          # 恢复余数，同时设置条件码
# andq %rdx, %rdx       # 重复操作，删除
jle Done               # 余数为 0，直接返回
```

这样余数为 0 时不进入尾部定位，尤其有利于 8 的倍数长度，结果为：

```text
Average CPE     7.74
Score   55.3/60.0
```

### 12.9 第七轮：BTFNT 分支预测

#### 先确认标准 PIPE 的预测规则

标准 PIPE 的取指预测通常写作：

```hcl
word f_predPC = [
    f_icode in { IJXX, ICALL } : f_valC;
    1 : f_valP;
];
```

`f_valC` 是指令中的目标地址，`f_valP` 是当前指令后面那条指令的地址。对 `IJXX` 无条件选择目标地址，表示 F 阶段总是先按“跳转会发生”取下一条指令，此时还没有在 E 判断条件。

预测的作用是选出接下来去哪里取指，不是提前修改条件码，也不是提前完成条件判断。

#### BTFNT 用地址方向决定预测

BTFNT 是 Backward Taken, Forward Not Taken：向低地址跳，预测跳转；向高地址跳，预测不跳转。当前代码比较目标地址与下一条地址：

```text
目标地址 < 下一条地址：预测跳转
目标地址 >= 下一条地址：预测不跳转
```

例如循环末尾通常向后跳回 `Loop`，预测跳转；正数判断的 `jle Next` 通常向前跳过 `count++`，预测不跳。这里“向后”“向前”说的是地址大小，不是条件成立与否。实际是否跳仍然取决于条件码。

无条件 `jmp` 和 `call` 应始终预测去目标地址，不能因为它们向前就预测不跳。当前 `f_predPC` 是：

```hcl
word f_predPC = [
    f_icode == ICALL : f_valC;
    f_icode == IJXX &&
        (f_ifun == 0 || f_valC < f_valP) : f_valC;
    1 : f_valP;
];
```

`ifun == 0` 对应无条件 `jmp`。HCL 的 `条件 : 值;` 是顺序选择关系，先匹配到哪一行就选哪一行；`&&` 则组成这一行的完整条件。

#### 预测变了，失败检测和恢复也要一起改

只改 F 的预测不够。标准的失败处理主要考虑“预测跳，实际不跳”；BTFNT 还增加了“预测不跳，实际跳”的可能，必须支持两种恢复地址。

当前使用已有信号传递两个地址，未修改 `Declarations` 部分：

| 地址 | D / E 中怎样传递 | M 中保存在哪里 |
| --- | --- | --- |
| 顺序执行地址 | `d_valA` 对 `IJXX` 选择 `D_valP`，之后随 `E_valA` 传递 | `M_valA` |
| 跳转目标地址 | E 阶段给 `IJXX` 选择 `aluA = E_valC`、`aluB = 0`，算出目标地址 | `M_valE` |

跳转借用 ALU 计算 `目标地址 + 0`，只是利用现有通路把目标传到 M，不代表跳转需要写通用寄存器或更新条件码。

分支在 E 阶段计算实际结果 `e_Cnd`。下面条件表示预测失败，并用于 `D_bubble` 和 `E_bubble` 的分支错误部分：

```hcl
E_icode == IJXX && E_ifun != 0 &&
((E_valC < E_valA && !e_Cnd) ||
 (E_valC >= E_valA && e_Cnd))
```

两类分别是：向后跳被预测为跳，但实际不跳；向前跳被预测为不跳，但实际跳。无条件 `jmp` 用 `E_ifun != 0` 排除。替换这部分时，保留原有加载冒险、`ret` 和异常相关控制。

F 根据 M 中已经保存的地址和实际分支结果恢复取指：

```hcl
word f_pc = [
    M_icode == IJXX && M_ifun != 0 &&
        M_valE < M_valA && !M_Cnd : M_valA;
    M_icode == IJXX && M_ifun != 0 &&
        M_valE >= M_valA && M_Cnd : M_valE;
    W_icode == IRET : W_valM;
    1 : F_predPC;
];
```

因此这一版仍然是 F 预测、E 判断、M 中的分支信息用于恢复取指。`M_Cnd` 保存的是那条分支实际是否成立的布尔结果，不是整套 ZF、SF、OF。

BTFNT 不是动态学习，也不保证每条分支都更准。元素的正负若变化较多，单纯用地址方向仍可能经常猜错；需要配合汇编布局，并通过测试比较。

加入 BTFNT 后记录为：

```text
Average CPE     7.71
Score   55.8/60.0
```

曾讨论把判断提前到 D，以减少预测失败损失；这需要额外考虑条件码何时可用、取指恢复和流水线清除，并受到声明区限制。本次最终 HCL 没有采用 D 提前判断，不应把前面某次移动 `andq`、存储指令的排布理解成处理器已经改成 D 判断。

### 12.10 第八轮：让余数为 1 时更快进入 Tail1

进入尾部时，`%rdx` 保存“真实余数 - 8”。旧做法先加 8 还原，再按 4、2 等基准细分：

```asm
iaddq $8, %rdx           # 假设剩 1 个：-7 + 8 = 1
jle Done
iaddq $-4, %rdx          # 1 - 4 = -3
jl TailSelectLow
# 后面还要加 2，再用 jl 跳到 Tail1
```

剩 1 个也要从 `Remain` 起执行 6 条定位指令。既然短数组的固定成本影响大，就先单独判断是不是 1 个：

```asm
iaddq $7, %rdx           # “余数 - 8”变成“余数 - 1”
jl Done                 # 余数为 0，结果为 -1
je Tail1                # 余数为 1，结果为 0
```

具体值如下：

| 真实余数 | 进入 Remain 时 `%rdx` | 加 7 后 | 路径 |
| --- | ---: | ---: | --- |
| 0 | -8 | -1 | `jl Done` |
| 1 | -7 | 0 | `je Tail1` |
| 2 | -6 | 1 | 继续细分 |
| 7 | -1 | 6 | 继续细分 |

这里不需要把 `%rdx` 恢复成真实余数，只需要维护“当前偏移了多少”的含义。若继续以 4 为比较基准，则减 3：`(余数 - 1) - 3 = 余数 - 4`。

后面的低数量分支也要同步简化：既然 0、1 已经处理，低数量区只剩 2、3，原来再次 `jl Tail1` 的判断就可以删除。

这次测试结果：

| 指标 | 修改前 | 修改后 |
| --- | ---: | ---: |
| Average CPE | 7.71 | 7.63 |
| Score | 55.8/60.0 | 57.3/60.0 |
| `len = 1` 的周期数 | 31 | 26 |

从 6 条定位指令变成 3 条，并不要求周期数也恰好少 3；分支预测成功或失败的变化也会影响流水线周期。YIS、PIPE 均 68/68 通过后保留这次修改。

### 12.11 第九轮：尾部布局、分组与预测代价一起比较

#### 把高数量判断挪到 Tail7 前面

原来的排列是先放高数量判断，再放低数量判断：

```asm
    # 先比较余数与 4
    jl TailSelectLow

    # 区分 4、5、6、7
    je Tail4
    # ...
    je Tail6
    jmp Tail7            # 需要跳过低数量判断代码

TailSelectLow:
    # 区分 2、3
    # ...

Tail7:
    # 处理第 7 个元素
```

`Tail7` 前面隔着低数量区，剩 7 个时不能直接往下执行，否则会误入另一段判断。把两段交换后：

```asm
    # 先比较余数与 4
    jge TailSelectHigh

    # 剩 2～3 个，直接顺序进入低数量判断
    # 判断后跳到 Tail2 或 Tail3
    # ...

TailSelectHigh:
    je Tail4
    # ...
    je Tail6
    # 其他情况一定是 7，不再执行 jmp Tail7
Tail7:
    # 直接顺序进入处理代码
```

判断的含义没变，只是把某个出口放在它要执行的代码前面。但入口条件从 `jl` 变成 `jge`，BTFNT 对这个向前条件跳转预测“不跳”，于是失败路径也变了：

| 余数 | 原布局：`jl TailSelectLow` | 新布局：`jge TailSelectHigh` |
| --- | --- | --- |
| 2～3 | 实际跳，预测失败 | 实际不跳，预测正确 |
| 4～7 | 实际不跳，预测正确 | 实际跳，预测失败 |

0、1 已在前面返回或跳走，不参与这次高低数量选择。省掉一条 `jmp` 的收益，可能被其他路径新增的预测失败抵消。实际从 7.63、57.3/60.0 变成 7.64、57.2/60.0，略微变慢；两项正确性仍均 68/68 通过。

这次修改说明：不能只比较静态代码少了几行，要比较各长度实际经过多少条指令、哪些条件跳转预测失败。

#### 不固定以 4 分界，比较 3、5 和不同布局

数量定位不一定非要在 4 分成两半。分组均匀，只代表最坏情况下判断层数可能比较均衡；它没有自动考虑分支失败代价，也没有考虑小长度对平均 CPE 的影响。

后续在临时文件中试了 7 个候选，保留同一套主循环、硬件和余数 0/1 快捷入口，只修改数量定位。以下是完整对比；其中“高、低数量在前”指代码排列位置，不是元素处理顺序：

| 布局 | Average CPE | Score |
| --- | ---: | ---: |
| 原来以 4 分界，高数量判断在前 | 7.63 | 57.3/60.0 |
| 以 4 分界，低数量判断在前，顺序进入 Tail7 | 7.64 | 57.2/60.0 |
| 以 3 为首个基准，直接区分 2/3、4/5、6/7 | **7.61** | **57.8/60.0** |
| 以 3 分组，低数量判断在前 | 7.63 | 57.3/60.0 |
| 以 3 分组，高数量判断在前 | 7.63 | 57.4/60.0 |
| 以 5 分组，低数量判断在前 | 7.62 | 57.5/60.0 |
| 以 5 分组，高数量判断在前 | 7.65 | 57.0/60.0 |
| 以 5 分组，低数量在前且先判断 4、再判断 3 | 7.65 | 57.1/60.0 |
| 以 5 分组，高数量在前且低数量区先判断 4、再判断 3 | 7.67 | 56.5/60.0 |

这些候选先比较性能，最后对最快的候选重新运行完整正确性与性能测试。相同的两位小数 CPE 可能有不同分数，因为脚本按未四舍五入的数值评分。

最终采用的代码是：

```asm
Remain:
    iaddq $7, %rdx       # 余数 - 1
    jl Done             # 剩 0 个
    je Tail1            # 剩 1 个

    iaddq $-2, %rdx      # 余数 - 3
    jl Tail2            # 已知至少 2 个，小于 3 就是 2
    je Tail3

    iaddq $-2, %rdx      # 余数 - 5
    jl Tail4            # 已知至少 4 个，小于 5 就是 4
    je Tail5

    iaddq $-1, %rdx      # 余数 - 6
    je Tail6
    # 否则一定剩 7 个，直接顺序进入 Tail7
Tail7:
    # 尾部复制与计数
```

它不再跳到单独的高低数量定位区，而是直接判断出某个数量就进入对应处理标签。整个控制过程变成：

```text
先处理 0、1
否则区分 2、3
否则区分 4、5
否则区分 6
否则直接处理 7
```

例如余数 5，进入 `Remain` 时 `%rdx = -3`，加 7 后为 4，第一次减 2 后为 2，第二次减 2 后为 0，于是 `je Tail5`。余数 7 则依次得到 6、4、2、1，所有条件都不成立，顺序进入 `Tail7`。

它并不是“判断次数最多的路径最少”的标准二分树，但在本次测试中，路径指令数、跳转数量、预测方向与平均 CPE 的组合更合适。

### 12.12 其他尝试：为什么没有一概采用

原始记录中的主线之外，还测试或讨论过下面这些方向。它们和主线所处的版本不一定相同，所以数值应当与各自当时的基线比较，不能直接当成同一轮横向实验：

| 尝试 | 记录与结论 |
| --- | --- |
| 主循环逐个安排、两组每组四个、四组每组两个 | 在当时八路版本中均约为 7.71；已有排布和转发能处理数据依赖，并不是增加寄存器就一定更快。当前保留四组、每组两个 |
| 十路展开 | 当时 7.69、56.2/60.0；代码 999 字节，已接近默认 1000 字节限制。后来按要求恢复八路 |
| 十一路展开 | 当时 7.68、56.5/60.0，但 1100 字节超过默认限制。放宽字节限制后功能通过，不代表满足正式限制，因此没有保留 |
| 条件传送替代正数判断分支 | 测过先准备候选 `count + 1`、再用 `cmovg` 选择的写法；BTFNT 下为 8.19、46.2/60.0。省掉分支，但需要复制、候选加法、测试和条件传送等额外指令，最终变慢 |
| 尾部直接由“余数 - 8”加 4 变成“余数 - 4” | 当时为 7.73、55.5/60.0，而原版本为 7.71；部分路径少了还原操作，但余数 0 不再那么早返回，其余定位路径也发生变化，最终恢复 |
| 尾部提前加载下一个元素 | 在当时 7.71 基线上测试为 7.78、54.4/60.0。虽然加载不改条件码，可以放到当前元素判断与跳转之间，但单独改造需要额外入口加载和跳转；现有转发又已消除加载到存储的暂停，所以没有独立收益，未保留 |

提前加载的思路仍值得理解。原来是：

```text
加载当前元素 → 存储 → 判断 → 条件跳转 → 可能 count++ → 再加载下一个
```

尝试改成：

```text
判断已加载的当前元素 → 存储当前元素 → 加载下一个 → 条件跳转 → 可能 count++
```

加载和存储都不修改条件码，所以跳转仍然判断当前元素；下一个标签可以直接使用已经加载的值。但第一个元素要从哪里加载、怎样进入相应标签，也必须算进成本。若为每个数量额外设置“先加载再跳转”的入口，节省的暂停可能为零，跳转反而增加。

若继续研究该方向，应把第一次加载与原有数量定位一起设计，并且只读取实际存在的元素，不能为了预加载而无条件越过有效数组范围。

此外，只把 `andq → jle` 中间插入存储，不会自动让分支在 D 判断。必须分别看当前 HCL 的判断阶段和数据路径，不能把汇编排布的变化与硬件功能的变化混为一谈。

### 12.13 当前版本的完整执行思路与性能进展

当前主循环的寄存器安排为：

| 寄存器 | 当前用途 |
| --- | --- |
| `%rax` | 正数个数，函数返回值；入口用 `xorq` 清零 |
| `%rdi` | 当前源块首地址 |
| `%rsi` | 当前目标块首地址 |
| `%rdx` | 主循环中是“预扣下一组 8 个后的长度”；尾部继续按不同基准偏移以定位数量 |
| `%r10`、`%r8` | 每组两个元素的临时值；尾部复用 `%r10` |

整个汇编执行过程是：

1. 返回值清零，先给长度减 8；不足 8 个直接进入尾部。
2. 主循环每轮处理 8 个元素，分四组，每组先加载两个，再分别复制、判断和计数。
3. 最后一组在完成加载后，源、目标指针各加 64；最后两个元素因此写到更新后目标指针的 `-16` 和 `-8` 偏移。
4. 每个元素都用自己的 `andq` 设置条件码，`jle` 跳过非正数的计数，正数通过 `iaddq $1, %rax` 加一。
5. 循环末尾再预扣 8，用 `jge Loop` 直接判断下一组是否足够。
6. 进入尾部后先加 7，从“余数 - 8”变成“余数 - 1”，快速处理 0、1，再依次区分 2/3、4/5、6/7。
7. 定位后从 `TailN` 连续执行到 `Tail1`，通过固定偏移复制和计数，不再更新指针与长度，最后 `ret`。

以 `len = 19` 为例，主循环处理 16 个后，源、目标指针都指向各自原数组的第 17 个元素，`%rdx = -5`。进入尾部后加 7 得到 2，再减 2 得到 0，直接进入 `Tail3`；依次处理剩余块偏移 16、8、0，也就是原数组第 19、18、17 个元素。

下面汇总原始笔记到当前版本的主线结果，原先每次的数值都保留：

| 阶段 | Average CPE | Score |
| --- | ---: | ---: |
| 初次重排依赖与入口 | 14.03 | 0.0/60.0 |
| 提取公共操作，删除重复 `andq` | 12.55 | 0.0/60.0 |
| 二路展开 | 11.78 | 0.0/60.0 |
| 把常数移到循环外 | 10.01 | 9.8/60.0 |
| 四路展开 | 9.10 | 28.0/60.0 |
| 八路展开 | 8.83 | 33.4/60.0 |
| 十六路展开试验，未保留 | 9.17 | 26.6/60.0 |
| 加载到存储转发后的当时记录 | 8.83 | 33.4/60.0 |
| PIPE 支持并使用 `iaddq` | 8.54 | 39.2/60.0 |
| 尾部按 4、2、1 分层 | 8.47 | 40.5/60.0 |
| 主循环预扣长度 | 8.22 | 45.5/60.0 |
| 余数定位后进入连续尾部标签 | 7.81 | 53.8/60.0 |
| 删除余数恢复后的重复判断，余数 0 尽早返回 | 7.74 | 55.3/60.0 |
| BTFNT | 7.71 | 55.8/60.0 |
| 余数 1 快捷入口，删除重复判断 | 7.63 | 57.3/60.0 |
| 调整高低数量布局以省掉 `jmp Tail7` | 7.64 | 57.2/60.0 |
| **按 2/3、4/5、6/7 顺序定位，当前采用** | **7.61** | **57.8/60.0** |

最后一次保留版本的验证命令和输出为：

```bash
# 在 archlab-handout/sim/pipe 中执行
./correctness.pl -q -f ncopy.ys
./correctness.pl -q -p -f ncopy.ys
./benchmark.pl -f ncopy.ys
```

```text
Simulating with instruction set simulator yis
68/68 pass correctness test
Simulating with pipeline simulator psim
68/68 pass correctness test

64      403     6.30
Average CPE     7.61
Score   57.8/60.0
```

这里 `len = 64` 的 CPE 是 6.30，而最终平均值是 7.61，两者不同正是因为评分还把短长度的各自 CPE 算进了平均。这些结果对应上面记录的当前版本；后续修改时，应按相同命令重新检查正确性和性能。
