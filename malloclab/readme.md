1 trace获取


2 认识当前情况

init free完全为空

`#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))`

这个宏的定义让我蒙蔽，

```c
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
```

不用细扣对齐数学原理，反正就是能直接 提升size到 %8==0的位置，并且这个位置 >= 原值，类似于向上取整

`#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))`，于是这个的意思就是将size_t向上取对齐位置

3 实现

根据阅读教材，可以实现一个 带头尾的堆，教材中有大量的宏定义，读起来我能明白意思，背着写对于我来说有一定难度，我是逐个方法实现，先看一个方法大致分为几步，然后逐步进行实现，在各步中实现所需要的宏

先完成init，init主要分为三步，扩充、填充头尾部、extend栈一个默认数4 KB，在栈上扩充单字（4字节）内存，然后word[0]对齐，word[1]放置最前面的头，word[2]放置最前面的尾，word[3]放置末尾的尾，这个主要是方便进行合并操作。前面操作完成两步，最后extend
