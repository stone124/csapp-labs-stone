This is an x86-64 bomb for self-study students.

# Bomb Lab 命令速查

下面的解压命令要在压缩包所在目录执行；解压后进入归档文件创建的目录，再运行后续命令。

## 解压与运行

```bash
tar -xvf bomb.tar
cd bomb
./bomb
```

如果归档解压出的目录名不是 `bomb`，将 `cd bomb` 中的目录名替换为实际名称。

也可以把已通过阶段的答案逐行保存在文本文件中，让 bomb 先读取文件，再从终端读取：

```bash
./bomb answers.txt
```

空行会被忽略。答案文件读到文件末尾后，程序会继续从标准输入读取。

## GDB 调试

```bash
gdb ./bomb
```

常用 GDB 命令：

| 命令                          | 用途                               |
| ----------------------------- | ---------------------------------- |
| `break main`                | 在`main` 函数设置断点            |
| `run`                       | 启动程序                           |
| `continue`                  | 继续运行到下一个断点               |
| `stepi` / `si`            | 单步执行一条汇编指令               |
| `nexti` / `ni`            | 单步执行，遇到函数调用时不进入函数 |
| `info registers`            | 查看寄存器                         |
| `x/<格式><数量><单位> 地址` | 查看指定地址的内存                 |
| `quit`                      | 退出 GDB                           |

## 导出汇编文件

```bash
objdump -d bomb > bomb_disassembly.txt
```

查阅工具帮助和手册：

```bash
apropos gdb
man gdb
info gdb
man ascii
info gas
```

Bomb Lab 的核心方法是阅读反汇编并用 GDB 跟踪执行、检查寄存器和内存；不要靠暴力猜答案。不同课程版本的阶段数和评分规则可能不同，具体以随 bomb 提供的 writeup 为准。
