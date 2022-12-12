# Quine-McCluskey Method
### Problem Description:
Implement a two-level logic optimizer based on Quine-McCluskey method. The first step is to generate all prime implicants for the given function. The second step is to choose a minimum-cost cover and generate a sum-of-products expression of this function with minimum number of prime implicant (If there exist different solutions with the same minimum number of prime implicant, please choose the one which has minimum number of literals).

### Input format: 
You should allow input from a file. <br> f(A,B,C,D) = Σ m(4,5,6,8,9,10,13) + Σ d(0,7,15). <br> 

The following is an example. 
```linux
.i  /* input variables */
4   /* A,B,C,D. */
.m  /* on set */
4 5 6 8 9 10 13
.d  /* don't care set */
0 7 15
```

### Output format:
Generate all prime implicants and the minimum sum-of-products expression to a file. <br> Prime implicants and minimum sum-of-products expression are separated by a line, and they should be sorted according to the order {-, 0, 1}. If there are more than 15 primary implicants, report only the first 15 primary implicants. However, you still have to report the correct number of total primary implicants at “.p” field.

The following is an example.
```linux
.p 7 /* there are 7 prime implicants */
-000 /* B'C'D' */
-1-1 /* BD */
0-00 /* A'C'D' */
01-- /* A'B */
1-01 /* AC'D */
10-0 /* AB'D'*/
100- /* AB'C' */
.mc 3 /* 3 prime implicants in minimum covering */
01-- /* A'B */
1-01 /* AC'D */
10-0 /* AB'D'*/
literal=8
```

### How to Compile
```linux
g++ -std=c++11 main.cpp -o main.exe
```
Generates executable binary file
### How to Execute
```linux
./main.exe <input file> <output file> 
```
Generates output file
