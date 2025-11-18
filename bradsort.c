/*
(Compact) Bitwise Radix Sort  (bradsort.c) (brs.c for *nix people) v1.50
Copyright 1991,1992,1993 by Edward Lee.  All Rights Reserved.

edlee@chinet.chinet.com

This program sorts strings with linear time complexity, O(N), when N is the 
length of the input data in bits (or in whatever units you choose, such as 
bytes or records, requiring a different hidden constant factor).  The bits 
of each string are represented as branches/edges/paths that grow from the 
root of a binary tree to the leaves of the tree [1].  A path that 
corresponds to a string always terminates at a node, but a node that 
terminates a path may or may not be a leaf node.  Starting with version 
1.3, only the minimal path needed to distinguish a string from any other 
string is inserted in the tree.  That is, only unique string prefixes (with 
bit granularity) are represented as paths in the tree, resulting in a more 
compact tree in most cases, generally saving memory space and speeding up 
the program by avoiding the work of allocating and initializing memory 
space for unnecessary nodes and branches.  This program is asymptotically 
faster than O(N*log(N)) sorting algorithms such as the well-known Quick 
Sort algorithm.  This means that as more and more data is processed, there 
will come a point when this program is faster than any Quick Sort 
implementation, and thereafter the average speed of this program relative 
to Quick Sort will continue to increase.  The main program currently uses a 
subset of the functions declared in this file for demonstration purposes, 
as a string sorting filter. 

Any data that can be represented as a string of displayable characters can 
be sorted in linear time with the functions in this program.  The most 
immediate example of this are the strings that you are now reading.

Raw binary data can also be sorted in linear time.  Floating-point numbers 
can be sorted after they are first converted to normalized 
representations.  Integers must be big-endian in order to be sorted; their 
byte order, their place values, must decrease as the byte address 
increases.  After unsigned integers are typecast into character arrays, 
unsigned integers can be sorted as general strings (see function insert(), 
below).  Signed integers have to be represented in such a way that their 
sign bits are in the most significant bit position and are 1 for positive 
integers and 0 for negative integers.  In other words, signed integers must 
be in excess notation.  Signed integers that are in two's complement 
notation can be converted to excess notation by executing an exclusive-or 
(XOR) operation on the sign bit with a corresponding 1, thus converting the 
signed integer into the format necessary for insertion into the radix tree.  
Characters are assumed by this program to be equivalent to bytes.

The speed of sorting will vary with the distribution of the data, though 
the worst-case performance is still in O(N) time.  When the string prefixes 
of the strings to be sorted are very different from each other, then the 
speed of sorting will be the fastest.  In other words, when the data is 
very sparse, the speed of sorting the data with this program will be the 
fastest.  This is because the height of the tree can be smaller, entailing 
fewer memory allocations for nodes and branches and shorter distances for 
tree traversal.  The sorting will also be fast when sparsely distributed 
strings are repeated many times, since this will not entail increasing the 
height of the tree.  When the string prefixes are almost entirely 
identical, almost to the entire lengths of the strings, then the speed of 
sorting will be the worst.  This is because the height of the tree will 
have to be bigger to create paths that represent mostly identical string 
prefixes, entailing more memory allocations and longer distances for tree 
traversal.  A Patricia Trie implementation can reduce this memory usage at 
the potential cost of reduced speed.

The speed of sorting will also depend on your computer's memory capacity 
and the speed of the C library of your compiler and the system-dependent 
code optimizations needed by this program to make your computer work at its 
fastest.  Main memory chip densities seem to be increasing and seem to be 
becoming more affordable aside from a temporary setback in resin 
production.  At least one new kind of 32 megabit chip is in development, 
and other memory technologies, such as holographic memory technologies and 
optical-chemical technologies, are in development.  When you have enough 
main memory to process all of your sorting jobs, then the cost of the 
memory will be a fixed cost and the cost of sorting will be a variable cost 
that this program or some variation of it may help to minimize.

This program now sorts a new string into the tree in constant time, O(1) 
time, for either a fixed-length string or a variable-length string that has 
a fixed maximum length.  This is achieved through the use of threads which 
directly link strings to each other.  Some situations in which this might 
be useful is in the maintenance of databases.  A common situation for a 
database is that it will be in a mostly sorted order or completely sorted 
order, and some new data will arrive to be incorporated into the database 
at irregular time intervals.  It seems that there are no other kinds of 
general sorting algorithms at present that allow O(1) time incremental 
sorting.  Some sorting algorithms based on balanced trees allow incremental 
sorting to occur in O(log(N)) time, but they are slower for a sufficiently 
large amount of data and may occasionally require rebalancing, which a trie 
does not require.

This program potentially operates in sub-linear space, Omega(1), due to the 
data compression that results from overlapping paths in the tree (which 
represent overlapping string prefixes) and the representation of string 
frequencies as numbers (up to the maximum unsigned integer representation) 
instead of as repetitions of the same string.  When using this program to 
determine the frequencies of words in large amounts of text, you are most 
likely to see sub-linear space usage.  On the other hand, there can be a 
large amount of memory overhead that approaches an upper bound of, 
approximately, the number of bits in a node/branch representation times 
the length of the input data.  The overlapping paths in the tree will not 
result in any sub-linear space usage for unique strings but will mitigate 
the amount of memory that is used, Theta(N) space.  In actual usage, you 
may see a memory overhead of 6 times the original size of the input data or 
less, depending on the lengths of overlapping string prefixes and the 
number of duplicate strings in the input data.

Nevertheless, a binary trie-based radix sort has the potential to be a 
winner in terms of both fast execution and efficient memory usage over 
other sorting algorithms.  A radix sort that is based on a trie operates in 
linear time regardless of the distribution of input strings.  Further, a 
bitwise radix sort is independent of the number of characters in the 
alphabet of the input strings.  The same can be said of other trie-based 
sorts that can be written, but they will generally require more memory for 
storing the tree structure, the tree traversal routines for them will be 
more complicated and can be slower, and converting different alphabets to 
them can take more time than converting alphabets to bitwise radix sorts 
since most digital computers already represent all of their data as binary 
code.

13 November 1993
*/
/*
[1] Two different people have commented to me that the tree structure 
reminds them of a trie, and I agree that that is what it is even though 
tries seem to lend themselves more to being represented as child-sibling 
trees instead of as non-child-sibling trees.  But the concept of finite 
state machines predates the coining of the term trie, and BRADSORT 
originally crystallized from the (perhaps inobvious) observation that a 
pre-order recursive traversal of a tree graph representing a particular 
finite state machine resulted in a sequence of paths that corresponded to 
the lexicographic ordering of the strings represented by the paths.

I knew about this when I was 16 or 17 years old in high school after 
misinterpreting brief descriptions of a different kind of radix sort on 
FidoNet and Usenet, the kind that you will currently find in books.  I 
pondered about the best data structure implementation for this type of tree 
for the application of sorting over several years before implementing the 
first version of BRADSORT.
*/
/*
 * Suggested compilation:
 *  Turbo C, Turbo C++, Borland C, Borland C++
 *     tcc -mc bradsort.c
 *  GNU C or DJGPP
 *     gcc -obradsort bradsort.c
 *  Unix Sys V
 *     cc -O -obradsort bradsort.c
 */
 
/*
 * Some possible invocations are:
 *      bradsort fur <InputFile
 * or
 *      bradsort fur <InputFile >OutputFile
 * or
 *      AnotherProgram | bradsort fur
 * or
 *      AnotherProgram | bradsort fur | YetAnotherProgram
 * or
 *      AnotherProgram | bradsort fur >OutputFile
 *
 * The options, f, u, and r (fur) stand for:
 *
 * (f)requency.  Show each string only once with its frequency next to it.
 * (u)nique.     Show each string only once.
 * (r)everse.    Show strings in reverse order.
 *
 * These options may be combined in any way.
 *
 * The default, if no options are specified, is to show the strings in
 * ascending order.
 */


/* v1.0, 1 October  1991 */
/*   This version was soon shown to Dr. Robert Kenyon to demonstrate that */
/* I did not have to take an introductory C programming course.  It */
/* sorted unique strings that terminated at leaves. */
/* v1.1, 2 November 1991 */
/*   This version handled non-unique strings and strings that did not */
/* necessarily terminate at leaves. */
/* v1.2 17 August 1992
 *   Changed the insert10() and newnode() routines so that the program
 * does not throw away all of its work when it is out of memory.
 * Modified the recursive string display routines to display all of the
 * strings contained in the radix tree instead of just the strings
 * that terminate at leaf nodes.
 */
/* 7 September 1992 */
/*   Split insert10() into two separate loops */
/* v1.3 24 February 1993 */
/*   Success. insert10() and showtree() are now modified to create */
/* and show a compact binary radix tree, respectively.  insert10() became */
/* more complicated, and showtree() became simpler.  The node representation */
/* also changed.  This version, in most cases, uses less memory and is */
/* faster than previous versions. */
/* 27 February 1993 */
/*   Added divergence10(), strlen10(), and used them to modify insert10() */
/* so that newstring10() memory allocations are not freed, to prevent memory */
/* allocation "holes" from developing.  This will simplify the development */
/* of a fast malloc() routine for use with this program.  Modified intree10() */
/* and delete10() and newnode() and showftree() and showutree() to handle the */
/* new tree structure. */
/* 2 March 1993 */
/*   Created versions of showtree(), showftree(), and showutree() for */
/* displaying the strings represented by the tree in reverse order */
/* 15 March 1993 */
/*   Added newstring() for the situation in which the length of the string */
/* is already known.  Added output buffering. */
/* 20 March 1993 */
/*   Added iterative traversal for special situations in showtree() */
/* 21 March 1993 */
/*   Added insert() for the new tree structure */
/* v1.4 22 March 1993 */
/*   Added intree() and delete() for the new tree structure */
/* Modified intoa() for handling just decimal ASCII representations */
/* 15 April 1993 v1.41 */
/*   Added more checks for out-of-memory conditions. Added binary file */
/* input/output for PC's.  Changed many char declarations to unsigned char. */
/* 24 April 1993 v1.42 */
/*   Made a slight optimization in the insert() and insert10() routines */
/* where some unnecessary indirection was taking place. */
/* 28 May 1993 v1.43 */
/*   Deleted redundant tests for null nodes in the show*() functions */
/* 31 July 1993 v1.44 */
/*   Added OutputString0() and OutputString() to shave off some more time */
/* that was spent in context switching and rearranged the output of the   */
/* string frequency displays for the sake of MSDOS systems.  Renamed some */
/* variables and show functions to better resemble the commandline:       */
/* showftree/showtree_f, showutree/showtree_u,  rshowtree/showtree_r,     */
/* rshowftree/showtree_rf, rshowutree/showtree_ru                         */
/* 1 August 1993 v1.45 */
/*   Added idivergence10() for more efficient string comparison when it */
/* is already known that two strings are identical up to a point.       */
/* 23 October 1993 */
/*   Changed many (uint) typecasts into (uchar). Began working on a     */
/* threaded tree implementation.                                        */
/* 24 October 1993 */
/*   Implemented find_predecessor(), find_successor(), link_predecessor(), */
/* link_successor(), and changed the string structure.                     */
/* 26 October 1993 */
/*   Implemented showlist(), showlist_f(), showlist_u(), showlist_r(), */
/* showlist_rf(), showlist_ru().                                       */
/* 30 October 1993 v1.50*/
/*   Modified insert() to handle threads. */
/*
I have encountered two kinds of faulty arguments that state that this 
bitwise radix sorting algorithm does not operate with linear time 
complexity or that this algorithm is slower than an O(N*log(N)) time 
sorting algorithm.

The first kind of argument asserts that the time complexity of this 
algorithm is O(N*log(N)) for unique data.  Someone in the comp.theory news 
group on Usenet incorrectly argued that ALL linear time sorting algorithms 
are really O(N*log(N)) time algorithms for unique data.  Here is the body 
of the article:

In article <1lckdiINNt4n@uwm.edu> markh@csd4.csd.uwm.edu (Mark) writes:
>   I don't know why people keep forgetting, but these algorithm[s] involve
>arithmetic manipulations on numeric variables whose values will range
>from 0 to N.  Arithmetic manipulations such as adding and incrementing are
>NOT constant time, but log(N), operations.  They only look constant because
>you're restricting yourself to a 32-bit set (which defeats the whole point
>of the term *asymptotic* complexity).
>
>   What they are, in fact, are N log(N) algorithms with VERY small coefficients.

Individual arithmetic manipulations such as shifting and adding do happen 
in constant time, because most computers today only perform arithmetic 
operations such as shifting and adding on a constant length of data.  When 
each individual string in the input data is longer than the computer's 
internal word size, the computer still only processes the string a constant 
length of data at a time.  The time required to process these individual 
constant lengths of data is constant and independent of the total length of 
the input data.

Consider a similar argument on an optimal comparison-based sorting 
algorithm that operates on random, unique, fixed-length keys.  A single 
comparison will require O(log(N)) time, a time proportional to the length 
of each key, for the number of keys, N, at its maximum.  The maximum number 
of comparisons on a single key is O(log(N)) comparisons.  So, the time that 
is required to process a single key is O(log(N)*log(N)) time.  For N keys, 
the time that is required to process them all is O(N*log(N)*log(N)) time.  
What is wrong with this argument is that the first log(N) is a constant 
that is being treated as if it were a variable.  The length of each key 
will remain constant even when the number of keys, N, is not at its 
maximum.

The second kind of argument is partially true.  Given a small N, this 
bitwise sorting algorithm, O(N) in time, is likely to be slower than a 
merge sort, O(N*log(N)) in time, because the hidden constant in O(N), let's 
call it C, will be larger than the log(N) times the hidden constant, let's 
call it L, in O(N*log(N)):

     C > L*log(N)

But, obviously, when N is sufficiently large:

     C < L*log(N)

So, the O(N) time sorting algorithm will finish faster than the O(N*log(N)) 
sorting algorithm for a sufficiently large N amount of data.

9 November 1993 Ed Lee
*/

#include <stdio.h>
#include <stdlib.h>
#ifdef __TURBOC__
#include <alloc.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
extern unsigned _stklen = 16384; /* Stack space */
#endif
#include <string.h>

#ifndef fputchar
#define fputchar(c) fputc(c, stdout)
#endif

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif

/* Despite what the ANSI C standard might tell you, chars are going to     */
/* be equivalent to bytes for a long time.  Too much code depends on this  */
/* in the absence of a standard alternative for specifying byte-length     */
/* variables. */

/* The maximum length, in bytes, of an input string.  You can modify this. */
#define MAXSLEN (255)
char s [MAXSLEN+1];
uint si = 0;

typedef struct string {
  uint frequency; /* Frequency of the string */
  uint len;       /* Length of the string */
  uchar *s;       /* Pointer to the string */
  struct string *previous_string; /* Pointer to in-order predecessor */
  struct string *next_string;     /* Pointer to in-order successor   */
} string;

typedef struct node {
  string *strinfo;     /* Pointer to information about a string */
  struct node *b [2];  /* Branches/Edges, indexed as 0 or 1 */
} node;

/* Bitmap for each possible character */
uint bitmap[256][8] = {
{0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,1}, {0,0,0,0,0,0,1,0}, {0,0,0,0,0,0,1,1}, 
{0,0,0,0,0,1,0,0}, {0,0,0,0,0,1,0,1}, {0,0,0,0,0,1,1,0}, {0,0,0,0,0,1,1,1}, 
{0,0,0,0,1,0,0,0}, {0,0,0,0,1,0,0,1}, {0,0,0,0,1,0,1,0}, {0,0,0,0,1,0,1,1}, 
{0,0,0,0,1,1,0,0}, {0,0,0,0,1,1,0,1}, {0,0,0,0,1,1,1,0}, {0,0,0,0,1,1,1,1}, 
{0,0,0,1,0,0,0,0}, {0,0,0,1,0,0,0,1}, {0,0,0,1,0,0,1,0}, {0,0,0,1,0,0,1,1}, 
{0,0,0,1,0,1,0,0}, {0,0,0,1,0,1,0,1}, {0,0,0,1,0,1,1,0}, {0,0,0,1,0,1,1,1}, 
{0,0,0,1,1,0,0,0}, {0,0,0,1,1,0,0,1}, {0,0,0,1,1,0,1,0}, {0,0,0,1,1,0,1,1}, 
{0,0,0,1,1,1,0,0}, {0,0,0,1,1,1,0,1}, {0,0,0,1,1,1,1,0}, {0,0,0,1,1,1,1,1}, 
{0,0,1,0,0,0,0,0}, {0,0,1,0,0,0,0,1}, {0,0,1,0,0,0,1,0}, {0,0,1,0,0,0,1,1}, 
{0,0,1,0,0,1,0,0}, {0,0,1,0,0,1,0,1}, {0,0,1,0,0,1,1,0}, {0,0,1,0,0,1,1,1}, 
{0,0,1,0,1,0,0,0}, {0,0,1,0,1,0,0,1}, {0,0,1,0,1,0,1,0}, {0,0,1,0,1,0,1,1}, 
{0,0,1,0,1,1,0,0}, {0,0,1,0,1,1,0,1}, {0,0,1,0,1,1,1,0}, {0,0,1,0,1,1,1,1}, 
{0,0,1,1,0,0,0,0}, {0,0,1,1,0,0,0,1}, {0,0,1,1,0,0,1,0}, {0,0,1,1,0,0,1,1}, 
{0,0,1,1,0,1,0,0}, {0,0,1,1,0,1,0,1}, {0,0,1,1,0,1,1,0}, {0,0,1,1,0,1,1,1}, 
{0,0,1,1,1,0,0,0}, {0,0,1,1,1,0,0,1}, {0,0,1,1,1,0,1,0}, {0,0,1,1,1,0,1,1}, 
{0,0,1,1,1,1,0,0}, {0,0,1,1,1,1,0,1}, {0,0,1,1,1,1,1,0}, {0,0,1,1,1,1,1,1}, 
{0,1,0,0,0,0,0,0}, {0,1,0,0,0,0,0,1}, {0,1,0,0,0,0,1,0}, {0,1,0,0,0,0,1,1}, 
{0,1,0,0,0,1,0,0}, {0,1,0,0,0,1,0,1}, {0,1,0,0,0,1,1,0}, {0,1,0,0,0,1,1,1}, 
{0,1,0,0,1,0,0,0}, {0,1,0,0,1,0,0,1}, {0,1,0,0,1,0,1,0}, {0,1,0,0,1,0,1,1}, 
{0,1,0,0,1,1,0,0}, {0,1,0,0,1,1,0,1}, {0,1,0,0,1,1,1,0}, {0,1,0,0,1,1,1,1}, 
{0,1,0,1,0,0,0,0}, {0,1,0,1,0,0,0,1}, {0,1,0,1,0,0,1,0}, {0,1,0,1,0,0,1,1}, 
{0,1,0,1,0,1,0,0}, {0,1,0,1,0,1,0,1}, {0,1,0,1,0,1,1,0}, {0,1,0,1,0,1,1,1}, 
{0,1,0,1,1,0,0,0}, {0,1,0,1,1,0,0,1}, {0,1,0,1,1,0,1,0}, {0,1,0,1,1,0,1,1}, 
{0,1,0,1,1,1,0,0}, {0,1,0,1,1,1,0,1}, {0,1,0,1,1,1,1,0}, {0,1,0,1,1,1,1,1}, 
{0,1,1,0,0,0,0,0}, {0,1,1,0,0,0,0,1}, {0,1,1,0,0,0,1,0}, {0,1,1,0,0,0,1,1}, 
{0,1,1,0,0,1,0,0}, {0,1,1,0,0,1,0,1}, {0,1,1,0,0,1,1,0}, {0,1,1,0,0,1,1,1}, 
{0,1,1,0,1,0,0,0}, {0,1,1,0,1,0,0,1}, {0,1,1,0,1,0,1,0}, {0,1,1,0,1,0,1,1}, 
{0,1,1,0,1,1,0,0}, {0,1,1,0,1,1,0,1}, {0,1,1,0,1,1,1,0}, {0,1,1,0,1,1,1,1}, 
{0,1,1,1,0,0,0,0}, {0,1,1,1,0,0,0,1}, {0,1,1,1,0,0,1,0}, {0,1,1,1,0,0,1,1}, 
{0,1,1,1,0,1,0,0}, {0,1,1,1,0,1,0,1}, {0,1,1,1,0,1,1,0}, {0,1,1,1,0,1,1,1}, 
{0,1,1,1,1,0,0,0}, {0,1,1,1,1,0,0,1}, {0,1,1,1,1,0,1,0}, {0,1,1,1,1,0,1,1}, 
{0,1,1,1,1,1,0,0}, {0,1,1,1,1,1,0,1}, {0,1,1,1,1,1,1,0}, {0,1,1,1,1,1,1,1}, 
{1,0,0,0,0,0,0,0}, {1,0,0,0,0,0,0,1}, {1,0,0,0,0,0,1,0}, {1,0,0,0,0,0,1,1}, 
{1,0,0,0,0,1,0,0}, {1,0,0,0,0,1,0,1}, {1,0,0,0,0,1,1,0}, {1,0,0,0,0,1,1,1}, 
{1,0,0,0,1,0,0,0}, {1,0,0,0,1,0,0,1}, {1,0,0,0,1,0,1,0}, {1,0,0,0,1,0,1,1}, 
{1,0,0,0,1,1,0,0}, {1,0,0,0,1,1,0,1}, {1,0,0,0,1,1,1,0}, {1,0,0,0,1,1,1,1}, 
{1,0,0,1,0,0,0,0}, {1,0,0,1,0,0,0,1}, {1,0,0,1,0,0,1,0}, {1,0,0,1,0,0,1,1}, 
{1,0,0,1,0,1,0,0}, {1,0,0,1,0,1,0,1}, {1,0,0,1,0,1,1,0}, {1,0,0,1,0,1,1,1}, 
{1,0,0,1,1,0,0,0}, {1,0,0,1,1,0,0,1}, {1,0,0,1,1,0,1,0}, {1,0,0,1,1,0,1,1}, 
{1,0,0,1,1,1,0,0}, {1,0,0,1,1,1,0,1}, {1,0,0,1,1,1,1,0}, {1,0,0,1,1,1,1,1}, 
{1,0,1,0,0,0,0,0}, {1,0,1,0,0,0,0,1}, {1,0,1,0,0,0,1,0}, {1,0,1,0,0,0,1,1}, 
{1,0,1,0,0,1,0,0}, {1,0,1,0,0,1,0,1}, {1,0,1,0,0,1,1,0}, {1,0,1,0,0,1,1,1}, 
{1,0,1,0,1,0,0,0}, {1,0,1,0,1,0,0,1}, {1,0,1,0,1,0,1,0}, {1,0,1,0,1,0,1,1}, 
{1,0,1,0,1,1,0,0}, {1,0,1,0,1,1,0,1}, {1,0,1,0,1,1,1,0}, {1,0,1,0,1,1,1,1}, 
{1,0,1,1,0,0,0,0}, {1,0,1,1,0,0,0,1}, {1,0,1,1,0,0,1,0}, {1,0,1,1,0,0,1,1}, 
{1,0,1,1,0,1,0,0}, {1,0,1,1,0,1,0,1}, {1,0,1,1,0,1,1,0}, {1,0,1,1,0,1,1,1}, 
{1,0,1,1,1,0,0,0}, {1,0,1,1,1,0,0,1}, {1,0,1,1,1,0,1,0}, {1,0,1,1,1,0,1,1}, 
{1,0,1,1,1,1,0,0}, {1,0,1,1,1,1,0,1}, {1,0,1,1,1,1,1,0}, {1,0,1,1,1,1,1,1}, 
{1,1,0,0,0,0,0,0}, {1,1,0,0,0,0,0,1}, {1,1,0,0,0,0,1,0}, {1,1,0,0,0,0,1,1}, 
{1,1,0,0,0,1,0,0}, {1,1,0,0,0,1,0,1}, {1,1,0,0,0,1,1,0}, {1,1,0,0,0,1,1,1}, 
{1,1,0,0,1,0,0,0}, {1,1,0,0,1,0,0,1}, {1,1,0,0,1,0,1,0}, {1,1,0,0,1,0,1,1}, 
{1,1,0,0,1,1,0,0}, {1,1,0,0,1,1,0,1}, {1,1,0,0,1,1,1,0}, {1,1,0,0,1,1,1,1}, 
{1,1,0,1,0,0,0,0}, {1,1,0,1,0,0,0,1}, {1,1,0,1,0,0,1,0}, {1,1,0,1,0,0,1,1}, 
{1,1,0,1,0,1,0,0}, {1,1,0,1,0,1,0,1}, {1,1,0,1,0,1,1,0}, {1,1,0,1,0,1,1,1}, 
{1,1,0,1,1,0,0,0}, {1,1,0,1,1,0,0,1}, {1,1,0,1,1,0,1,0}, {1,1,0,1,1,0,1,1}, 
{1,1,0,1,1,1,0,0}, {1,1,0,1,1,1,0,1}, {1,1,0,1,1,1,1,0}, {1,1,0,1,1,1,1,1}, 
{1,1,1,0,0,0,0,0}, {1,1,1,0,0,0,0,1}, {1,1,1,0,0,0,1,0}, {1,1,1,0,0,0,1,1}, 
{1,1,1,0,0,1,0,0}, {1,1,1,0,0,1,0,1}, {1,1,1,0,0,1,1,0}, {1,1,1,0,0,1,1,1}, 
{1,1,1,0,1,0,0,0}, {1,1,1,0,1,0,0,1}, {1,1,1,0,1,0,1,0}, {1,1,1,0,1,0,1,1}, 
{1,1,1,0,1,1,0,0}, {1,1,1,0,1,1,0,1}, {1,1,1,0,1,1,1,0}, {1,1,1,0,1,1,1,1}, 
{1,1,1,1,0,0,0,0}, {1,1,1,1,0,0,0,1}, {1,1,1,1,0,0,1,0}, {1,1,1,1,0,0,1,1}, 
{1,1,1,1,0,1,0,0}, {1,1,1,1,0,1,0,1}, {1,1,1,1,0,1,1,0}, {1,1,1,1,0,1,1,1}, 
{1,1,1,1,1,0,0,0}, {1,1,1,1,1,0,0,1}, {1,1,1,1,1,0,1,0}, {1,1,1,1,1,0,1,1}, 
{1,1,1,1,1,1,0,0}, {1,1,1,1,1,1,0,1}, {1,1,1,1,1,1,1,0}, {1,1,1,1,1,1,1,1}
};  /* bitmap[256][8] */

node *root;
node *choppoint;
uint  chopdir;
string empty_string;

uchar OutputBuffer[32768];
uint BufferIndex = 0;

/* Buffer output characters */
void OutputChar(ch)
  uchar ch;
{
  OutputBuffer[BufferIndex++] = ch;
  if (BufferIndex == sizeof(OutputBuffer)) {
     fwrite(OutputBuffer, sizeof(OutputBuffer), 1, stdout);
     BufferIndex = 0;
  }
} /* OutputChar */

/* Buffer output strings that are \0-terminated */
void OutputString0(s)
 char *s;
{
  while (*s != '\0') {
    OutputBuffer[BufferIndex++] = *s++;
    if (BufferIndex == sizeof(OutputBuffer)) {
       fwrite(OutputBuffer, sizeof(OutputBuffer), 1, stdout);
       BufferIndex = 0;
    }
  }
}  /* OutputString0 */

/* Buffer output strings when we know the length */
void OutputString(s, len)
 char *s;
 uint len;
{
  while (len--) {
    OutputBuffer[BufferIndex++] = *s++;
    if (BufferIndex == sizeof(OutputBuffer)) {
       fwrite(OutputBuffer, sizeof(OutputBuffer), 1, stdout);
       BufferIndex = 0;
    }
  }

  OutputBuffer[BufferIndex++] = '\n';  /* Add a line feed to the output */
  if (BufferIndex == sizeof(OutputBuffer)) {
     fwrite(OutputBuffer, sizeof(OutputBuffer), 1, stdout);
     BufferIndex = 0;
  }
}  /* OutputString */

#define OUTFLUSH fwrite(OutputBuffer, BufferIndex, 1, stdout)

/* Compare two strings, returning the displacement where the two strings */
/* first diverge.   If the returned offset is equal to the lengths */
/* of both strings, then the strings are identical. */
uint idivergence10(s1, s1index, s2)
  uchar  *s1;
  uint    s1index;
  string *s2;
{
uint len2;
uchar *c2;

  len2 = s2->len - s1index;
  c2   = s2->s   + s1index;

  while (*s1 != '\n' && len2 != 0) {
        if (*s1++ != *c2++) break;
        --len2;
  }

  return (s2->len - len2);
}  /* idivergence10() */

/*
    strlen10() returns the length of a \n\0-terminated string that has 
    been created by fgets(), not including the '\n'. This gambles on 
    strlen() being optimized for the particular system on which this 
    program is compiled.
*/
#define strlen10(s) ( (uint)strlen( (char *)s) - 1U)

/* Function to convert an integer to ASCII text in base 10 */
void intoa(s, i)
 uchar *s;
 uint i;
{
static uchar a[] = {'0','1','2','3','4','5','6','7','8','9'};
static uchar temp[65];
uchar * tp=temp;
uint len=0, r;

  /* Get digits from right to left, low-order to high-order */
  do {
    r = i % 10;                  /* Get rightmost digit      */
    *tp++ = a[r];                /* Store a[digit] in temp[] */
    ++len;                       /* Keep track of length     */
    i = i / 10;                  /* Chop off rightmost digit */
  } while (i);                   /* Do this until i==0       */

  while (len--) *s++ = *(--tp);  /* Copy temporary string in reverse */

  *s = '\0';                     /* Null-terminate string */
}  /* intoa */


/* Display a critical error and exit the program */
void error (s)
 char *s;
{
  (void)fputs(s, stderr);
  (void)fputs("\n", stderr);
  exit(1);
} /* error */


/* Outputs all of the strings in the tree in ascending order */
void showlist()
{
string *p = &empty_string;
uint frequency;

  empty_string.previous_string->next_string = NULL;

  while (p != NULL) {  /* Checks for NULL are faster on many systems */
        frequency = p->frequency;
        while (frequency--)
              OutputString(p->s, p->len);
        p = p->next_string;
  }
  
  empty_string.previous_string->next_string = &empty_string;
}  /* showlist */

/* Outputs the unique strings of the tree */
void showlist_u()
{
string *p = &empty_string;

  empty_string.previous_string->next_string = NULL;

  if (p->frequency == 0)
     p = p->next_string;

  while (p != NULL) {
        OutputString(p->s, p->len);
        p = p->next_string;
  }

  empty_string.previous_string->next_string = &empty_string;
}  /* showlist_u */

/* Outputs the strings of the tree alongside their frequencies */
void showlist_f ()
{
string *p = &empty_string;

  empty_string.previous_string->next_string = NULL;

  if (p->frequency == 0)
     p = p->next_string;

  while (p != NULL) {
        intoa(s, p->frequency);
        OutputString0(s);
        OutputChar(' ');
        OutputString(p->s, p->len);
        p = p->next_string;
  }

  empty_string.previous_string->next_string = &empty_string;
}  /* showlist_f */

/* Outputs all of the strings in the tree in reverse */
void showlist_r()
{
uint frequency;
string *p;
string *temp;

  temp = p = empty_string.previous_string;

  empty_string.previous_string = NULL;

  while (p != NULL) {
        frequency = p->frequency;
        while (frequency--)
              OutputString(p->s, p->len);
        p = p->previous_string;
  }
  
  empty_string.previous_string = temp;
}  /* showlist_r */

/* Outputs the unique strings of the tree in reverse */
void showlist_ru()
{
string *p = empty_string.previous_string;

  empty_string.next_string->previous_string = NULL;

  while (p != NULL) {
        OutputString(p->s, p->len);
        p = p->previous_string;
  }

  if (empty_string.frequency != 0)
     OutputString(empty_string.s, 0);

  empty_string.next_string->previous_string = &empty_string;
}  /* showlist_ru */

/* Outputs the unique strings of the tree in reverse alongside their frequencies */
void showlist_rf()
{
string *p = empty_string.previous_string;

  empty_string.next_string->previous_string = NULL;

  while (p != NULL) {
        intoa(s, p->frequency);
        OutputString0(s);
        OutputChar(' ');
        OutputString(p->s, p->len);
        p = p->previous_string;
  }

  if (empty_string.frequency != 0) {
     intoa(s, p->frequency);
     OutputString0(s);
     OutputChar(' ');
     OutputString(p->s, p->len);
  }

  empty_string.next_string->previous_string = &empty_string;
}  /* showlist_rf */


/* Creates string information for a string of a known length */
string *newstring (s, len)
 uchar *s;
 uint len;
{
  uchar *t=s;
  string *p;

  if ( (p = (string *) malloc (sizeof(string) ) ) != NULL) {
     p->frequency = 1;
     p->len  = len;
     if ( (t = p->s = (uchar *) malloc (len) ) == NULL) return (string *)NULL;
     while (len--) *t++ = *s++;  /* Copy the string */
  }

  return p;
}  /* newstring() */

/* Creates string information for a newline-terminated string */
string * newstring10 (s)
 uchar * s;
{
  uchar *t=s;
  string *p;
  uint len;

  len = 0;
  while (*t++ != '\n') ++len;  /* Count up to a delimiter */

  if ( (p = (string *) malloc (sizeof(string) ) ) != NULL) {
     p->frequency = 1;
     p->len = len;
     if ( (t = p->s = (uchar *) malloc (len) ) == NULL) return (string *)NULL;
     while (len--) *t++ = *s++;  /* Copy the string */
  }

  return p;
}  /* newstring10() */


/* Creates a new node for the compact binary radix tree structure */
node * newnode () {
node *p;

  if ( (p = (node *) malloc (sizeof(node) ) ) != NULL) {
     p->strinfo = NULL;
     p->b[0] = NULL;
     p->b[1] = NULL;
  } else {  /* We are out of memory, so let's clean up what we've inserted */
     p = choppoint->b[chopdir];
     choppoint->b[chopdir] = NULL;  /* Chop off the branch */

     while ((choppoint = p) != NULL) {  /* Remove the branch */
           if (p->b[0] != NULL)
              p = p->b[0];
           else
              p = p->b[1];  

           if (choppoint->strinfo != NULL)
              free(choppoint->strinfo);

           free(choppoint);
     }  /* while */
     p = NULL;
  }
  return p;
}  /* newnode() */


/* Links a new string, p2, into the thread when an existing p1 that is */
/* already in the thread is known */
void link_predecessor(p1, p2)
  string *p1;
  string *p2;
{
string *p3;

  p3 = p1->next_string;

  p2->previous_string = p1;
  p2->next_string     = p3;

  if (p1 != NULL)
     p1->next_string = p2;
  
  if (p3 != NULL)
     p3->previous_string = p2;
}  /* link_predecessor */


/* Links a new string, p2, into the thread when an existing p3 that is */
/* already in the thread is known */
void link_successor(p2, p3)
  string *p2;
  string *p3;
{
string *p1;

  p1 = p3->previous_string;
  
  p2->previous_string = p1;
  p2->next_string     = p3;

  if (p1 != NULL)
     p1->next_string = p2;

  if (p3 != NULL)  
     p3->previous_string = p2;
}  /* link_successor */


/* Returns a pointer to an in-order predecessor string */
string *find_predecessor(p)
  node *p;
{
  do {
    if (p->b[1] != NULL)
       p = p->b[1];
    else
       if (p->b[0] != NULL)
          p = p->b[0];
       else
          break;
  } while (1);

  return p->strinfo;
}  /* find_predecessor */


/* Returns a pointer to an in-order successor string */
string *find_successor(p)
  node *p;
{
  do {
    if (p->strinfo != NULL)
       return p->strinfo;

    if (p->b[0] != NULL)
       p = p->b[0];
    else
       if (p->b[1] != NULL)
          p = p->b[1];
  } while (1);
}  /* find_successor */


/* Inserts a newline-terminated string into the tree */
uint insert10 (s)
 uchar * s;
{
uchar * b = s;  /* Pointer to the beginning of the string */
string *currentstr, *temp;
uint bit, i, len;
uint j=0;  /* Index to the character currently being inserted in the tree */
uint d;
node *p;

  choppoint = NULL;
  p = root;

  /* Follow the existing path until the path ends or the string terminates, */
  /* whichever comes first */

  while (*s != '\n') {

    for (bit=0; bit<8; ++bit) {
        i = bitmap [(uchar)*s] [bit];

        if (p->b[i] == NULL)  { /* The path ended */
           choppoint = p;
           chopdir   = i;

           if (p->strinfo == NULL) {  /* Is string information already here? */
              if ( (p->b[i] = newnode() ) == NULL) return 1;  /* No */
              /* Point to string information */
              if ( (p->b[i]->strinfo = newstring10(b) ) == NULL) return 1;
              if (i == 0)
                 link_successor (p->b[0]->strinfo, find_successor(p->b[1]) );
              else
                 link_predecessor (find_predecessor(p->b[0]), p->b[1]->strinfo);
              return 0;
           } else {                   /* Yes */
              /* Synchronize strings until there is a divergence */
              d = idivergence10 (s, j, p->strinfo);

              if (b[d] == '\n' && d == p->strinfo->len) {
                 p->strinfo->frequency += 1; /* Update existing string information */
                 return 0;
              }

              if ( (currentstr = newstring10(b) ) == NULL) return 1;
              
              temp = p->strinfo; /* Keep track of the existing string info */
              p->strinfo = NULL; /* Prepare to move the existing string info */

              /* Is the current character less than the diverging character? */
              if (j < d) {
                 /* Insert the rest of the current character */
                 do {
                     if ( (p->b[i] = newnode() ) == NULL) return 1;
                     p = p->b[i];
                     ++bit;
                     if (bit == 8) break;
                     i = bitmap [(uchar)*s] [bit];
                 } while (1);

                 /* Insert any intervening characters */
                 ++s;
                 ++j;

                 while (j < d) {
                     for (bit=0; bit<8; ++bit) {
                         i = bitmap [(uchar)*s] [bit];
                         if ( (p->b[i] = newnode() ) == NULL) return 1;
                         p = p->b[i];
                     }  /* for */

                     ++s;
                     ++j;
                 }  /* while */

                 bit = 0;
              }  /* if */

              if (currentstr->len == d) {  /* Only one branch diverges */
                 p->strinfo = currentstr;
                 i=bitmap[(uchar)(temp->s[d])] [0];
                 if ( (p->b[i] = newnode() ) == NULL) return 1;
                 p->b[i]->strinfo = temp;
                 link_successor (currentstr, temp);
                 return 0;
              }

              if (temp->len == d) {        /* Only one branch diverges */
                 p->strinfo = temp;
                 i=bitmap[(uchar)*s] [0];
                 if ( (p->b[i] = newnode() ) == NULL) return 1;
                 p->b[i]->strinfo = currentstr;
                 link_predecessor (temp, currentstr);
                 return 0;
              }

              /* Two branches diverge */

              /* Synchronize any matching prefix bits at the character */
              /* where the strings diverge */
              while ((i=bitmap[(uchar)*s] [bit]) ==
                        bitmap[(uchar)(temp->s[d]) ] [bit]) {
                    if ( (p->b[i] = newnode() ) == NULL) return 1;
                    p = p->b[i];
                    ++bit;
              }

              /* Insert the diverging branches */
              i=bitmap[(uchar)*s] [bit];
              if ( (p->b[i] = newnode() ) == NULL) return 1;
              p->b[i]->strinfo = currentstr;

              i = i ^ 0x1;              /* Index to the opposite branch */
              if ( (p->b[i] = newnode() ) == NULL) return 1;
              p->b[i]->strinfo = temp;

              if (i==0)
                 link_predecessor (temp, currentstr);
              else
                 link_successor (currentstr, temp);

              return 0;
           }  /* if */
        }  /* if */

        p = p->b[i];
    } /* for */

    ++s;  /* Point to the next character */
    ++j;
  }  /* while */

  /* The string terminated */

  /* Create string information if it does not already exist here */
  if (p->strinfo == NULL) {
     if ( (p->strinfo = newstring10(b) ) == NULL) return 1;

     /* Link p->strinfo with in-order successor */
     if (p->b[0] != NULL)
        link_successor (p->strinfo, find_successor(p->b[0]) );
     else
        link_successor (p->strinfo, find_successor(p->b[1]) );

     return 0;
  }

  /* String information already exists here */

  len = strlen10(b);
  if (p->strinfo->len > len) {
     temp = p->strinfo;  /* A longer string is already represented here */
     if ( (p->strinfo = newstring(b, len) ) == NULL) return 1;
     i = bitmap [(uchar)temp->s [len] ] [0];
     if ( (p->b[i] = newnode() ) == NULL) return 1;
     p->b[i]->strinfo = temp;

     link_successor (p->strinfo, temp);

     return 0;
  }
  
  p->strinfo->frequency += 1;  /* Signal the termination of the string */
  return 0;
}  /* insert10() */


#ifdef EXTRA
/* Outputs all of the strings in the tree */
void showtree (p)
 node *p;
{
uint frequency, i;

  if (p == NULL) return;

  if (p->strinfo != NULL) {  /* Show the string */
     frequency = p->strinfo->frequency;
     while (frequency--)
           OutputString(p->strinfo->s, p->strinfo->len);
  }

  /* The following loop iteratively traverses paths that have no */
  /* forks so that some stack space and time spent in context switching */
  /* can be saved.  This loop can be deleted, and this function */
  /* will still work correctly, although more slowly and with */
  /* more stack thrashing.  Writing similar loops for the other */
  /* show functions is left as an exercise for the reader. Be careful. ;-) */
  do {
     if (p->b[0] != NULL && p->b[1] == NULL)
        p = p->b[0];
     else
        if (p->b[0] == NULL && p->b[1] != NULL)
           p = p->b[1];
        else
           break;

     if (p->strinfo != NULL) {  /* Show the string */
        frequency = p->strinfo->frequency;
        while (frequency--)
              OutputString(p->strinfo->s, p->strinfo->len);
     } /* if */
  } while (1);  /* Infinite loop exited by a break */

  /* Recursive traversal */  
  showtree(p->b[0]);

  showtree(p->b[1]);
} /* showtree */

/* Outputs the unique strings of the tree */
void showtree_u (p)
 node *p;
{
uint i;

  if (p == NULL) return;

  if (p->strinfo != NULL)
     OutputString(p->strinfo->s, p->strinfo->len);

  showtree_u(p->b[0]);

  showtree_u(p->b[1]);
}  /* showtree_u */

/* Outputs the strings of the tree alongside their frequencies */
void showtree_f (p)
 node *p;
{
uint i;

  if (p == NULL) return;

  if (p->strinfo != NULL) {  
     intoa(s, p->strinfo->frequency);
     OutputString0(s);
     OutputChar(' ');
     OutputString(p->strinfo->s, p->strinfo->len);
  }

  showtree_f(p->b[0]);

  showtree_f(p->b[1]);
}  /* showtree_f */

/* Outputs all of the strings in the tree in reverse */
void showtree_r (p)
 node *p;
{
uint frequency, i;

  if (p == NULL) return;

  showtree_r(p->b[1]);

  showtree_r(p->b[0]);

  if (p->strinfo != NULL) {  
     frequency = p->strinfo->frequency;
     while (frequency--)
           OutputString(p->strinfo->s, p->strinfo->len);
  }
}  /* showtree_r */

/* Outputs the unique strings of the tree in reverse */
void showtree_ru (p)
 node *p;
{
uint i;

  if (p == NULL) return;

  showtree_ru(p->b[1]);

  showtree_ru(p->b[0]);

  if (p->strinfo != NULL)
     OutputString(p->strinfo->s, p->strinfo->len);
}  /* showtree_ru */

/* Outputs the strings of the tree alongside their frequencies, in reverse */
void showtree_rf (p)
 node *p;
{
uint i;

  if (p == NULL) return;

  showtree_rf(p->b[1]);

  showtree_rf(p->b[0]);

  if (p->strinfo != NULL) {  
     intoa(s, p->strinfo->frequency);
     OutputString0(s);
     OutputChar(' ');
     OutputString(p->strinfo->s, p->strinfo->len);
  }
}  /* showtree_rf */

uint divergence(s1, len1, s2)
  uchar *s1;
  uint len1;
  string *s2;
{
uint len2 = s2->len;
uchar *c2 = s2->s;

  while (len1!=0 && len2!=0) {
        if (*s1++ != *c2++) break;
        --len1;
        --len2;
  }

  return (s2->len - len2);
}  /* divergence() */


/* Inserts a general string, one that can contain '\0', into the tree */
uint insert (s, len)
 uchar * s;
 uint len;  /* Length of s in characters */
{
uchar *b = s;  /* Pointer to the beginning of the string */
string *currentstr, *temp;
uint bit, i;
uint slen = len;
uint j=0;  /* Index to the character currently being inserted in the tree */
uint d;
node *p;

  choppoint = NULL;
  p = root;

  /* Follow the existing path until the path ends or the string terminates, */
  /* whichever comes first */

  while (slen--) {

    for (bit=0; bit<8; ++bit) {
        i = bitmap [(uchar)*s] [bit];

        if (p->b[i] == NULL)  { /* The path ended */
           choppoint = p;
           chopdir   = i;

           if (p->strinfo == NULL) {  /* Is string information already here? */
              if ( (p->b[i] = newnode() ) == NULL) return 1;  /* No */
              /* Point to string information */
              if ( (p->b[i]->strinfo = newstring(b, len) ) == NULL) return 1;
              if (i == 0)
                 link_successor (p->b[0]->strinfo, find_successor(p->b[1]) );
              else
                 link_predecessor (find_predecessor(p->b[0]), p->b[1]->strinfo);

              return 0;
           } else {                   /* Yes */
              /* Synchronize strings until there is a divergence */
              d = divergence (b, len, p->strinfo);

              if (d == len && d == p->strinfo->len) {
                 p->strinfo->frequency += 1; /* Update existing string information */
                 return 0;
              }

              if ( (currentstr = newstring(b, len) ) == NULL) return 1;
              
              temp = p->strinfo; /* Keep track of the existing string info */
              p->strinfo = NULL; /* Prepare to move the existing string info */

              /* Is the current character less than the diverging character? */
              if (j < d) {
                 /* Insert the rest of the current character */
                 do {
                     if ( (p->b[i] = newnode() ) == NULL) return 1;
                     p = p->b[i];
                     ++bit;
                     if (bit == 8) break;
                     i = bitmap [(uchar)*s] [bit];
                 } while (1);

                 /* Insert any intervening characters */
                 ++s;
                 ++j;

                 while (j < d) {
                     for (bit=0; bit<8; ++bit) {
                         i = bitmap [(uchar)*s] [bit];
                         if ( (p->b[i] = newnode() ) == NULL) return 1;
                         p = p->b[i];
                     }  /* for */

                     ++s;
                     ++j;
                 }  /* while */

                 bit = 0;
              }  /* if */

              if (currentstr->len == d) {  /* Only one branch diverges */
                 p->strinfo = currentstr;
                 i=bitmap[(uchar)(temp->s[d])] [0];
                 if ( (p->b[i] = newnode() ) == NULL) return 1;
                 p->b[i]->strinfo = temp;
                 link_successor (currentstr, temp);
                 return 0;
              }

              if (temp->len == d) {        /* Only one branch diverges */
                 p->strinfo = temp;
                 i=bitmap[(uchar)*s] [0];
                 if ( (p->b[i] = newnode() ) == NULL) return 1;
                 p->b[i]->strinfo = currentstr;
                 link_predecessor (temp, currentstr);
                 return 0;
              }

              /* Two branches diverge */

              /* Synchronize any matching prefix bits at the character */
              /* where the strings diverge */
              while ((i=bitmap[(uchar)*s] [bit]) ==
                        bitmap[(uchar)(temp->s[d]) ] [bit]) {
                    if ( (p->b[i] = newnode() ) == NULL) return 1;
                    p = p->b[i];
                    ++bit;
              }

              /* Insert the diverging branches */
              i=bitmap[(uchar)*s] [bit];
              if ( (p->b[i] = newnode() ) == NULL) return 1;
              p->b[i]->strinfo = currentstr;
              
              i ^= 0x1;                 /* Index to the opposite branch */
              if ( (p->b[i] = newnode() ) == NULL) return 1;
              p->b[i]->strinfo = temp;

              if (i==0)
                 link_predecessor (temp, currentstr);
              else
                 link_successor (currentstr, temp);

              return 0;
           }  /* if */
        }  /* if */

        p = p->b[i];
    } /* for */

    ++s;  /* Point to the next character */
    ++j;
  }  /* while */

  /* The string terminated */

  /* Create string information if it does not already exist here */
  if (p->strinfo == NULL) {
     if ( (p->strinfo = newstring(b, len) ) == NULL) return 1;

     /* Link p->strinfo with in-order successor */
     if (p->b[0] != NULL)
        link_successor (p->strinfo, find_successor(p->b[0]) );
     else
        link_successor (p->strinfo, find_successor(p->b[1]) );

     return 0;
  }

  /* String information already exists here */

  if (p->strinfo->len > len) {
     temp = p->strinfo;  /* A longer string is already represented here */
     if ( (p->strinfo = newstring(b, len) ) == NULL) return 1;
     i = bitmap [(uchar)temp->s [len] ] [0];
     if ( (p->b[i] = newnode() ) == NULL) return 1;
     p->b[i]->strinfo = temp;

     link_successor (p->strinfo, temp);

     return 0;
  }
  
  p->strinfo->frequency += 1;  /* Signal the termination of the string */
  return 0;
}  /* insert() */


/* Compare two strings, returning the displacement where the two strings */
/* first diverge.   If the returned offset is equal to the lengths */
/* of both strings, then the strings are identical. */
uint divergence10(s1, s2)
  uchar *s1;
  string *s2;
{
uint len2 = s2->len;
uchar *c2 = s2->s;

  while (*s1 != '\n' && len2 != 0) {
        if (*s1++ != *c2++) break;
        --len2;
  }

  return (s2->len - len2);
}  /* divergence10() */


/* A function that indicates the frequency of a particular general
 * string in the tree
 */
uint intree (s, len)
 uchar *s;
 uint len;  /* Length of s in characters */
{
uchar * b = s;  /* Pointer to the beginning of the string */
node * p;
uint d, i;
uint slen=len;

  p = root;

  while (slen--) {
    i = bitmap [(uchar)*s] [0];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    i = bitmap [(uchar)*s] [1];
    if (p->b[i] == NULL)  break;
    p = p->b[i];
       
    i = bitmap [(uchar)*s] [2];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    i = bitmap [(uchar)*s] [3];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    i = bitmap [(uchar)*s] [4];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    i = bitmap [(uchar)*s] [5];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    i = bitmap [(uchar)*s] [6];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    i = bitmap [(uchar)*s] [7];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    ++s;  /* Point to next character */
  }  /* while */

  if (p->strinfo == NULL) return 0;

  d = divergence (b, len, p->strinfo);
  if (d == len && p->strinfo->len == d)
     return p->strinfo->frequency;

  return 0;
}  /* intree() */

/* A function that indicates the frequency of a particular newline-terminated
 * string in the tree
 */
uint intree10 (s)
 uchar *s;
{
uchar * b = s;  /* Pointer to the beginning of the string */
node * p;
uint d, i;

  p = root;

  while (*s != '\n') {
    i = bitmap [(uchar)*s] [0];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    i = bitmap [(uchar)*s] [1];
    if (p->b[i] == NULL)  break;
    p = p->b[i];
       
    i = bitmap [(uchar)*s] [2];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    i = bitmap [(uchar)*s] [3];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    i = bitmap [(uchar)*s] [4];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    i = bitmap [(uchar)*s] [5];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    i = bitmap [(uchar)*s] [6];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    i = bitmap [(uchar)*s] [7];
    if (p->b[i] == NULL)  break;
    p = p->b[i];

    ++s;  /* Point to next character */
  }  /* while */

  if (p->strinfo == NULL) return 0;

  d = divergence10 (b, p->strinfo);
  if (b[d] == '\n' && p->strinfo->len == d)
     return p->strinfo->frequency;

  return 0;
}  /* intree10() */


/* A function that deletes a particular general string from the tree */
void delete (s, len)
 uchar *s;
 uint len;  /* Length of s in characters */
{
uchar * b = s;  /* Points to the beginning of the string */
node * p;
uint d, i;
uint chopdir;
uint slen = len;
node *choppoint = root;  /* Node of previous branch or logical end of string */

  if (len == 0) return;

  p = root;

  do {
    i = bitmap [(uchar)*s] [0];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    i = bitmap [(uchar)*s] [1];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];
       
    i = bitmap [(uchar)*s] [2];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    i = bitmap [(uchar)*s] [3];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    i = bitmap [(uchar)*s] [4];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    i = bitmap [(uchar)*s] [5];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    i = bitmap [(uchar)*s] [6];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    i = bitmap [(uchar)*s] [7];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    ++s;  /* Point to next character */
  }  while (--slen);

  /* Is the string under consideration not in the tree? */
  if (p->strinfo == NULL)
     return;  /* Yes, consider it deleted */

  d = divergence (b, len, p->strinfo);
  if (len != d || p->strinfo->len != d)
     return;  /* Yes, consider it deleted */

  /* Does a longer string overlap the one under consideration? */
  if ((p->b[0] != NULL) || (p->b[1] != NULL)) {
     p->strinfo->frequency = 0;  /* Yes, delete the substring */
     /* free(p->strinfo);  p->strinfo=NULL; */ /* A slower alternative */
     return;
  }

  /* No, prepare to delete down from the choppoint */
  p = choppoint->b [chopdir];
  choppoint->b [chopdir] = NULL;  /* Cut off the branch */

  /* p now points to the cut off branch, a (meandering) singly linked list */
  while ((choppoint = p) != NULL) {

        if (p->b[0] != NULL)
           p = p->b[0];
        else
           p = p->b[1];  

        if (choppoint->strinfo != NULL)
           free(choppoint->strinfo);

        free(choppoint);
  }  /* while */
 
}  /* delete() */

/* A function that deletes a particular newline-terminated
 * string from the tree
 */
void delete10 (s)
 uchar *s;
{
uchar * b = s;  /* Points to the beginning of the string */
node * p;
uint d, i;
uint chopdir;
node *choppoint = root;  /* Node of previous branch or logical end of string */

  if (*s == '\n') return;

  p = root;

  do {
    i = bitmap [(uchar)*s] [0];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    i = bitmap [(uchar)*s] [1];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];
       
    i = bitmap [(uchar)*s] [2];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    i = bitmap [(uchar)*s] [3];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    i = bitmap [(uchar)*s] [4];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    i = bitmap [(uchar)*s] [5];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    i = bitmap [(uchar)*s] [6];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    i = bitmap [(uchar)*s] [7];
    if (p->b[i] == NULL)  break;
    if ((p->b[i ^ 0x1] != NULL) || (p->b[i]->strinfo != NULL)) {
       choppoint = p;
       chopdir = i;
    }
    p = p->b[i];

    ++s;  /* Point to next character */
  }  while (*s != '\n');

  /* Is the string under consideration not in the tree? */
  if (p->strinfo == NULL)
     return;  /* Yes, consider it deleted */

  d = divergence10 (b, p->strinfo);
  if (b[d] != '\n' || p->strinfo->len != d)
     return;  /* Yes, consider it deleted */

  /* Does a longer string overlap the one under consideration? */
  if ((p->b[0] != NULL) || (p->b[1] != NULL)) {
     p->strinfo->frequency = 0;  /* Yes, delete the substring */
     /* free(p->strinfo);  p->strinfo=NULL; */ /* A slower alternative */
     return;
  }

  /* No, prepare to delete down from the choppoint */
  p = choppoint->b [chopdir];
  choppoint->b [chopdir] = NULL;  /* Cut off the branch */

  /* p now points to the cut off branch, a (meandering) singly linked list */
  while ((choppoint = p) != NULL) {

        if (p->b[0] != NULL)
           p = p->b[0];
        else
           p = p->b[1];  

        if (choppoint->strinfo != NULL)
           free(choppoint->strinfo);

        free(choppoint);
  }  /* while */
 
}  /* delete10() */
#endif


int main (paramcount, paramstr)
  int paramcount;
  char *paramstr[];
{
char *ch;
uint i;
uint options=0;
uint incomplete;

#ifdef __TURBOC__
unsigned long mem1, mem2;

  mem1=farcoreleft();

  (void)setmode(fileno(stdin), O_BINARY);
  (void)setmode(fileno(stdout), O_BINARY);

  fprintf(stderr, "BRADSORT v1.50 Copyright 1993 by Edward Lee, All Rights Reserved.\n");
  fprintf(stderr, "Far core left: %lu\n", mem1);
#endif

  /* Process command line options */
  for (i=1; i<paramcount; ++i) {
     ch = paramstr[i];
     while (*ch) {
       if (*ch == 'u' || *ch == 'U') options |= 0x1;
       else
       if (*ch == 'r' || *ch == 'R') options |= 0x2;
       else
       if (*ch == 'f' || *ch == 'F') options |= 0x4;
       ++ch;
     }
  }

  root = newnode();
  root->strinfo = &empty_string;
  root->strinfo->frequency = 0;
  root->strinfo->len       = 0;
  root->strinfo->s         = NULL;
  root->strinfo->previous_string = &empty_string;
  root->strinfo->next_string     = &empty_string;

  while (NULL != fgets (s, MAXSLEN+1, stdin))
        if ((incomplete=insert10 ((uchar *)s))==1) break;

#ifdef __TURBOC__
  mem2=farcoreleft();
#endif

  switch (options) {
    /* Show all of the strings */
/*    case 0: showtree(root);   break;*/
    case 0: showlist(); break;

    /* Show all unique strings */
/*    case 1: showtree_u(root);  break;*/
    case 1: showlist_u(); break;

    /* Show all of the strings in reverse */
/*    case 2: showtree_r(root);  break;*/
    case 2: showlist_r();  break;

    /* Show all unique strings in reverse */
/*    case 3: showtree_ru(root); break;*/
    case 3: showlist_ru(); break;

    /* Show all unique strings alongside their frequencies */
    case 4:
/*    case 5: showtree_f(root);  break;*/
    case 5: showlist_f();  break;

    /* Show all unique strings & their frequencies in reverse */
    case 6:
/*    case 7: showtree_rf(root); break;*/
    case 7: showlist_rf(); break;
  }

  OUTFLUSH;

#ifdef __TURBOC__
  fprintf(stderr, "Far core left: %lu\n", mem2);
  fprintf(stderr, "Far core used: %lu\n", mem1-mem2);
#endif

  if (incomplete) error("The sorting is incomplete.");

  return 0;
}  /* main() */
