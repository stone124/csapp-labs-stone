/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in（提交）to your instructor（授课教师）.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses（使困惑） the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression（表达式） using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary（一元的） integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict（限制） the set of allowed operators（运算符） even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros（宏）.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies（意味着） that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts（右移） arithmetically（算术方式）.
  3. Has unpredictable（不可预测的） behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary（任意的） integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler（编译器） (described in the handout（实验说明）) to
     check the legality（合规性） of your solutions（解答）.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment（赋值） ('=') is not counted; you may use as many of
     these as you want without penalty（惩罚）.
  3. Use the btest test harness（测试框架） to check your functions for correctness（正确性）.
  4. Use the BDD checker（BDD 检查器） to formally verify（形式化验证） your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  /*转换问题：找到x y不同的地方标记为1，例如0 1为1，1 0为1
  one_same_mark代表x和y同时为1的地方标记为1，
  zero_same_mark代表x和y同时为0的地方标记为1，
  one_same_mark取反代表x和y同时为0 或者 不同的地方 标记为1，
  zero_same_mark取反代表x和y同时为1 或者 不同的地方 标记为1，
  ~one_same_mark & ~zero_same_mark代表不同的地方，就是解答的问题
  */

  int one_same_mark = x & y;
  int zero_same_mark = ~x & ~y;
  return ~one_same_mark & ~zero_same_mark;
}
/* 
 * tmin - return（返回） minimum（最小的） two's complement（二进制补码） integer（整数）
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  /*转换问题：最高位为1，其他位为0*/
  int number = 1;

  return number << 31;
}
//2
/*
 * isTmax - returns（返回） 1 if x is the maximum（最大值）, two's complement（二进制补码） number（数）, 
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  /*问题转换：当0x7fffffff的时候返回1，否则返回0，意思是我需要一个位运算，与输入作用得到0x00000000、0x00000001
  每一位的值都限定死了，需要运算直接判断每一位的值，例如&可以是1&1才能为1，如果就采取这个&，光移位就会有32次。不可行

  0xefffffff难以处理，取反之后为 0x80000000，只有最高位为1，更加方便处理，左移一位就会变为0，能满足这些要求的只有0x7fffffff和0xffffffff,
  与0x80000000取&就可以直接判断出最高位是0和1，然后就需要排除其他最高位为0的数，0x7fffffff和0xffffffff还有个共同特征（其他最高位为0所不具有的），x + 1 与 ~x 相同。
  总之，两个条件x + 1 与 ~x 相同，然后最高位为0，这两个的直接结果都是0，取反取&即可

  没有移位操作，难以判断最高位为0，于是单独排除-1（0xffffffff）,它的特征就是取反为0，通过两个!转换为1

  tip: 0x00000000可以通过逻辑运算！变为1，非零值（包含负数），可变换为0
  tip：相等可以用异或^判断
  */
  int not = ~x;
  int xplus = x + 1;
  int condition1_equal = !(not ^ xplus);

  return condition1_equal & !!not;
}
/* 
 * allOddBits - return（返回） 1 if all odd-numbered（编号为奇数的） bits（位） in word（字） are set to 1
 *   where bits are numbered（编号） from 0 (least significant（最低有效位）) to 31 (most significant（最高有效位）)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  /*首先构造所有奇数位是1的数作为掩码，用x&掩码，如果说x所有odd位都是1的话，
  假设结果为result_mask，则result_mask与odd_pos_mask_32是一致的，
  可以用^异或判断相等*/
  int odd_pos_mask_2 = 2; 
  int odd_pos_mask_4 = (odd_pos_mask_2 << 2) + odd_pos_mask_2;
  int odd_pos_mask_8 = (odd_pos_mask_4 << 4) + odd_pos_mask_4;
  int odd_pos_mask_16 = (odd_pos_mask_8 << 8) + odd_pos_mask_8;
  int odd_pos_mask_32 = (odd_pos_mask_16 << 16) + odd_pos_mask_16;

  int result_mask = odd_pos_mask_32 & x;
  int equal = !(result_mask^odd_pos_mask_32);
  return equal;
}
/* 
 * negate - return（返回） -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  /*首先观察-x与x的关系，x取反+1，要问咋来的，正数补码是本身，负数补码是取反+1*/
  return (~x)+1;
}
//3
/* 
 * isAsciiDigit - return（返回） 1 if 0x30 <= x <= 0x39 (ASCII codes（ASCII 编码） for characters（字符） '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  /*思路1：
  可以用上个方法的取负方法，将0x30的负值和0x39负值分别与x相加，如果说0x39负值与x的结果小于等于0，
  0x30的负值与x的结果大于等于0，则可以认为是 1。

  但是等于0就有点碍眼了，于是可以计算0x39负值与x-1(小于0)，然后0x30负值与x+1(大于0)*/
  // int negative_0x30 = (~0x30)+1;
  // int negative_0x39 = (~0x39)+1;
  // int negative_1 = (~1)+1;
  // int result_0x30 = negative_0x30 + x + 1;
  // int result_0x39 = negative_0x39 + x + negative_1;
  // return !!result_0x30 & !!result_0x39;
  // 0x80000000出现超限，不可行

  /*思路2：
  0x30和0x39他们的高位部分是相同的，然后低位需要在0-9之间。

  首先，右边移4位看是否与3相等
  然后，取出低位部分，与0-9做差，也就是负值相+，观察符号位
  */
  int x_high_pos = x >> 4;
  int high_pos_equal = !(x_high_pos ^ 3);
  int low_pos_mask = 0x0000000f;
  int x_low_pos = low_pos_mask&x;
  int is_low_pos = (x_low_pos+ ~1 + ~9+2) >>31;

  return high_pos_equal & is_low_pos;
}
/* 
 * conditional - same as（等价于） x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  /*判断x的值非0，然后如果非0选择y，如果说为0选择z。
  也就是用一个0 1值去与y z做运算，可以让y z分别去与掩码进行运算，
  掩码0xffffffff和0x0的值就差1，刚好适合这个0 1值进行运算*/
  int is_x_0 = !x;
  int mask = 0;
  int z_mask = mask + ~is_x_0 + 1;
  int y_mask = mask + is_x_0 + ~1 + 1;

  return (y&y_mask) + (z&z_mask);
}
/* 
 * isLessOrEqual - if x <= y then return（返回） 1, else（否则） return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  /*需要判断x <= y
  还是通过x的相反数进行加y，但是x也只是为负数的话，这样转换并不是对应的正数，只有正数转换为负数才是。
  即使只看大小关系，正数和负数之间也难以计算。
  可以分为三种情况，1 负数和正数 2 正数和正数 3 负数与负数 4 正数与负数
  简单来说就是 符号相同时，比较差值，符号不同时，直接比较符号。
  符号相同时，如何比较差值呢，就通过掩码把除符号位的位保留，然后做差，比大小（用符号位判断）
  
  tip: 求差后，符号位0 1，可判断大于等于情况，求差后!，可判断相等情况*/
  int sign_x = x >> 31;
  int sign_y = y >> 31;
  int diff_sign = sign_y + ~sign_x + 1;
  int same_sign_or_x_negative = diff_sign >> 31; // y_sign >= x_sign，为0
  int same_sign = !diff_sign; // x和y的符号位相同时，为1

  int value_mask = ~(1 << 31);
  int value_x = value_mask & x;
  int value_y = value_mask & y;
  int diff_value = value_y + ~value_x + 1;
  int sign_diff_value = (diff_value >> 31); // 符号位为0，即为x_value <= y_value

  return (!same_sign_or_x_negative & !same_sign) | (same_sign & !sign_diff_value); // 用|所有返回1的情况：1 y为正数、x为负数 2 y和x符号相同，但是x_value <= y_value
}
//4
/* 
 * logicalNeg - implement（实现） the ! operator（运算符）, using（使用） all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  return 2;
}
/* howManyBits - return（返回） the minimum（最少） number of bits（位） required to represent（表示） x in
 *             two's complement（二进制补码）
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  return 0;
}
//float
/* 
 * floatScale2 - Return（返回） bit-level equivalent（位级等价结果） of expression（表达式） 2*f for
 *   floating point（浮点数） argument（参数） f.
 *   Both the argument（参数） and result（结果） are passed as unsigned int's（无符号整数）, but
 *   they are to be interpreted（解释） as the bit-level representation（位级表示） of
 *   single-precision（单精度） floating point（浮点数） values（值）.
 *   When argument（参数） is NaN（非数）, return（返回） argument（参数）
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  return 2;
}
/* 
 * floatFloat2Int - Return（返回） bit-level equivalent（位级等价结果） of expression（表达式） (int) f
 *   for floating point（浮点数） argument（参数） f.
 *   Argument（参数） is passed as unsigned int（无符号整数）, but
 *   it is to be interpreted（解释） as the bit-level representation（位级表示） of a
 *   single-precision（单精度） floating point（浮点数） value（值）.
 *   Anything out of range（超出范围） (including（包括） NaN（非数） and infinity（无穷大）) should return（返回）
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  return 2;
}
/* 
 * floatPower2 - Return（返回） bit-level equivalent（位级等价结果） of the expression（表达式） 2.0^x
 *   (2.0 raised to the power（的 x 次幂） x) for any 32-bit integer（32 位整数） x.
 *
 *   The unsigned（无符号） value（值） that is returned（返回） should have the identical（相同的） bit
 *   representation（表示） as the single-precision（单精度） floating-point（浮点数） number（数） 2.0^x.
 *   If the result（结果） is too small to be represented（表示） as a denorm（非规格化数）, return（返回）
 *   0. If too large, return +INF（正无穷大）.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
    return 2;
}
