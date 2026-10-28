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

## part c

这里优化可以认为是两个方面，1 减少指令量 2 优化流水线

优化流水线可以分为两个方面，1 减少前后两条指令依赖，这样流水线就可以打满

2 减小条件预测失败的代价，让条件更容易预测

先找指令依赖，这很明显

Loop:	mrmovq (%rdi), %r10	# read val from src...
	rmmovq %r10, (%rsi)	# ...and store it to dst

这里和上一条指令形成了依赖，必须等上条走完memory阶段，这一条才能走decode，可以提前进行位置加减，这样就能打满流水线，把常数提前存到r11

就可以变成这个样子

Loop:	mrmovq (%rdi), %r10	# read val from src...
	irmovq $8, %r11
	addq %r11, %rdi		# src++
	addq %r11, %rsi		# dst++
	rmmovq %r10, -8(%rsi)	# ...and store it to dst

rsi地址需要进行变换，一个元素8个字节

然后，条件预测更容易，就是把条件判断改为经常成立的一条
andq %rdx,%rdx		# len <= 0?

这个很明显，大多数调用情况都是len > 0，改为 len > 0更合理
andq %rdx,%rdx		# len > 0?
	jg Loop		# if so, goto Loop:
	ret # if not, ret
先测试一版，

make -B psim VERSION=full CFLAGS="-Wall -O2 -std=gnu89 -fcommon" GUIMODE= TKLIBS= TKINC=

./correctness.pl -f ncopy.ys       # 在 ISA 模拟器上检查汇编逻辑
./correctness.pl -p -f ncopy.ys    # 在 PIPE 模拟器上检查

两项都通过后，测性能：

./benchmark.pl -f ncopy.ys

Average CPE     14.03
Score   0.0/60.0

看看能不能减小指令量

jle Npos		# if so, goto Npos:
	irmovq $1, %r10
	addq %r10, %rax		# count++
Npos:	irmovq $1, %r10

这里无论走哪一个分支都要irmovq $1, %r10，，可以提前irmovq $1, %r10

subq %r10, %rdx		# len--
	andq %rdx,%rdx		# len > 0?
	jg Loop			# if so, goto Loop:

去掉andq的条件判断效果是一样的

再测试一下

Average CPE     12.55
Score   0.0/60.0

好了一些，还是挺拉跨的。

只好改变算法逻辑了，在程序优化这章讲了循环展开方法，可以尝试下，也就是一次处理多个元素，也就是把计算链路拆成两条并行的。

拆成两行并行，首先最开始len > 1，两个val值得分别判断，最后还要加个remain循环

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

改完之后变为

Average CPE     11.78
Score   0.0/60.0

再进行精简下指令

有很多常数赋值，把他们统一挪到前面去

irmovq $1, %r9 # 常数1, %r9
	irmovq $2, %rcx # 常数2, %rcx
	irmovq $16, %r11 # 常数地址挪2，也就是16字节

测试结果为

Average CPE     10.01
Score   9.8/60.0

尝试四路展开，结果如下

Average CPE     9.10
Score   28.0/60.0

尝试8路展开，结果如下

Average CPE     8.83
Score   33.4/60.0

尝试16路展开，结果如下

Average CPE	9.17
Score	26.6/60.0

8路展开是性能最好的，保持8路展开。
