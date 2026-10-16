# 2026秋季CSAPP实验记录

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
