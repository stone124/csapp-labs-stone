# 2026秋季CSAPP实验记录

## Lab 1 — Data Lab: Manipulating Bits

## 实验结果

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

## 解题小提示

- 逻辑非 `!` 可以把 `0` 转换为 `1`，把任意非零值（包括负数）转换为 `0`。
- 判断两个值是否相等时，可以利用异或：相同的值异或结果为 `0`。
- 全 1 掩码 `0xffffffff` 与全 0 掩码只相差 `1`，可以利用这个关系，把 `0` 或 `1` 转换为按位选择所需的掩码。
- 两数做差后，可以结合差值的符号位判断大小；逻辑非 `!` 可用于判断差值是否为 `0`。
- 对负数进行算术右移时，左侧补入符号位 `1`，所以结果的最高位仍为 `1`。

详细的解题注释和代码位于 `datalab-handout/bits.c`，写在各函数定义后的函数体开头注释中。
