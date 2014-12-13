/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
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

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

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

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
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
/* 
 * evenBits - return word with all even-numbered bits set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 1
 */
int evenBits(void) {
    int a=0x55;
    a=a<<8|a;
    a=a<<16|a;	    
    return a;
}
/* 
 * isEqual - return 1 if x == y, and 0 otherwise 
 *   Examples: isEqual(5,5) = 1, isEqual(4,5) = 0
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int isEqual(int x, int y) {
    return !(x^y); 
}
/* 
 * byteSwap - swaps the nth byte and the mth byte
 *  Examples: byteSwap(0x12345678, 1, 3) = 0x56341278
 *            byteSwap(0xDEADBEEF, 0, 2) = 0xDEEFBEAD
 *  You may assume that 0 <= n <= 3, 0 <= m <= 3
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 2
 */
int byteSwap(int x, int n, int m) {
    /*int a,b,mask,c;
    n<<=3;
    m<<=3;
    a=x>>n;
    b=x>>m;
    mask=0xff;
    a=a&mask;
    b=b&mask;
    a<<=m;
    b<<=n;
    b=b|a;
    c=(mask<<n)|(mask<<m);
    c=~c;
    x&=c;
    x|=b;
    return x;*/
    int a,b;
    int mask=0xff;
    m<<=3;
    n<<=3;
    a=((x>>m)^(x>>n))&mask;
    b=(a<<m)|(a<<n);
    return x^b;
}
/* 
 * rotateRight - Rotate x to the right by n
 *   Can assume that 0 <= n <= 31
 *   Examples: rotateRight(0x87654321,4) = 0x18765432
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 25
 *   Rating: 3 
 */
int rotateRight(int x, int n) {
/*    int a=x>>n;
    int b=(~n)+33;
    int mask=(~0)<<b;
    a=a&(~mask);
    return a|(x<<b);
*/
    int b=~n+32;
    return (((x>>31)^x)<<b<<1)^(x>>n);
}
/* 
 * logicalNeg - implement the ! operator using any of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
/*
    int a=~x+0x01;	//2's compliment
    x^=a;
    x|=a;
    x>>=31;
    return x+0x01;
*/
    int a=~x+1;
    x|=a;
    return (x>>31)+1;
}
/* 
 * TMax - return maximum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmax(void) {
    int a=0x01;
    return ~(a<<31);
}
/* 
 * sign - return 1 if positive, 0 if zero, and -1 if negative
 *  Examples: sign(130) = 1
 *            sign(-23) = -1
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 10
 *  Rating: 2
 */
int sign(int x) {
    int a=x>>31;
    int b=!!x;
    return a|b;
}
/* 
 * isGreater - if x > y  then return 1, else return 0 
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
    int k=x^y;
    int m=~y+x;
    return (((k&y)|(~(k|m)))>>31)&1;
}
/* 
 * subOK - Determine if can compute x-y without overflow
 *   Example: subOK(0x80000000,0x80000000) = 1,
 *            subOK(0x80000000,0x70000000) = 0, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int subOK(int x, int y) {
    /*int k=(x^y)>>31;
    y=~y+0x01;
    y+=x;
    return (!k)|(((k&(x^y))>>31)+0x01);*/
    int m=~y+x+1;
    int k=x^y;
    return (((k&(x^m)))>>31)+1;
}
/*
 * satAdd - adds two numbers but when positive overflow occurs, returns
 *          maximum possible value, and when negative overflow occurs,
 *          it returns minimum possible value.
 *   Examples: satAdd(0x40000000,0x40000000) = 0x7fffffff
 *             satAdd(0x80000000,0xffffffff) = 0x80000000
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 30
 *   Rating: 4
 */
int satAdd(int x, int y) {
    int m=x+y;
    int a=((x^m)&(y^m))>>31; 
    //return (~a&b)|(a&(b>>31^(1<<31)));
    return (m>>(a&31))^(a<<31);
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
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
    /*int a=x>>31;
    int d1,d2,d3,d4,d56;
    int k,l,n;
    x^=a;
    d1=(x>>30)&1;
    d2=(!d1)&(!!(x>>14));
    d3=(!d1)&((!d2&(!!(x>>16)))|(!!(x>>23)));
    d4=(!!(x>>26))|((!d3)&(!!(x>>18)))|((!d2)&(!!(x>>10)))|((!d2)&(!d3)&(!!(x>>2)));
    d4=(!d1)&d4;
    k=((!(((x^1)&((x>>4)^1)&((x>>8)^1)&((x>>12)^1)&((x>>16)^1)&((x>>20)^1)&((x>>24)^1)&((x>>28)^1))))<<1);
    l=((~!!(((x>>1)&(x>>5)&(x>>9)&(x>>13)&(x>>17)&(x>>21)&(x>>25)&(x>>29))^1))<<1)&0x3;
    //m=((x>>2)&(x>>6)&(x>>10)&(x>>14)&(x>>18)&(x>>22)&(x>>26)&(x>>30))^1;
    n=!!((((x>>3)^1)&((x>>7)^1)&((x>>11)^1)&((x>>15)^1)&((x>>19)^1)&((x>>23)^1)&((x>>27)^1)));
    d56=(((!d1)<<1)+!d1)&(k|l|n);
    return ((d1<<5)+(d2<<4)+(d3<<3)+(d4<<2)+d56);*/
    
    int a;
    x=x^(x>>31);
    a=(!!(x>>16))<<4;
    a=a+((!!(x>>(8+a)))<<3);
    a=a+(((!(x>>(4+a)))<<2)^5);
    a=a+((!!(x>>(1+a)))<<1);
    a=a+(x>>a);
    return a+(!!x);
}
/* 
 * float_half - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_half(unsigned uf) {
    /*unsigned s=uf&0x80000000;
    unsigned exp=(uf&0x7fffffff)>>23;
    unsigned frac=uf&0x7fffff;
    
    if(exp==0xff){
	//special
	return uf;
    }else if(exp>1){
	//normal
	exp-=1;
    }else{
	if((frac&3)>2)
	    frac+=1;

	frac=(frac>>1)+(exp<<22);
	exp=0;
    }
	
    return s+(exp<<23)+frac;*/
    unsigned s=uf&0x80000000;
    unsigned exp=uf&0x7fffffff;
    //unsigned frac=uf&0x7fffff;
/*
    if(exp==0x7f800000){
	return uf;
    }else if(exp>0x800000){
	exp-=0x800000;
    }else{
	if((frac&3)>2)
	    frac+=1;

	frac=(frac+exp)>>1;
	exp=0;
    }
*/
    if(exp>=0x7f800000){
	return uf;
    }else if(exp>=0x1000000){
	exp-=0x800000;
    }else{
	if((exp&3)>2)
	    exp+=1;

	exp>>=1;
    }
    return s+exp;
}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
    unsigned s=uf>>31;
    unsigned exp=(uf&0x7fffffff)>>23;
    unsigned frac=uf&0x7fffff;
    unsigned m=frac+0x800000;
    unsigned e,ans;

    if(exp>157)
	return 0x80000000;
    else if(exp<127)
	return 0;
    else{
	e=157-exp;
	m<<=7;
	ans=m>>e;
    }
    
    if(s){
	return -ans;
    }else return ans;

}
