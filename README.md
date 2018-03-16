# Optimized-Radix-Sort
An optimized implementation of the Radix LSD sorting algorithm for integers, written in C.

<h2><b>Algorithm:</b></h2>
<p>Radix Sort is a non-comparative sorting algorithm able to sort a list of N integers in O(kn) time complexity.
<br>The presented program showcases an implementation of said algorithm written in ANSI C and optimized for speed.</p>
<p>The function sorts 32-bit integers (signed or unsigned), one byte at a time (a bit like the American Flag sorting,
<br>but starting with the Least Significant Digit/Byte).</p>
<p>This implementation follows an out-of-place approach, meaning it uses an helper array in order to successfully sort
  the original vector of integers.</p>
<p>The program supports all main C compilers (GCC, Clang, MCVS, ...) and architectures.
<p>For more information about the algorithm itself check the
<a href="https://en.wikipedia.org/wiki/Radix_sort">Wikipedia Page</a>.</p>

<h2><b>Optimizations:</b></h2>

<p>The sorting function was made without the help of compiler otimizations in mind.
<br>Therefore, it uses some microtimizations like macros and registers to speed some things up.
<br>Be mindful that compilers like GCC don't provide optimizations unless told otherwise.
<br>The ideia is to just include the function in your program and get the performance right out
of the box.</p>

<ul>
  <li>Use of powers of 2 for the expoents and bucket size in order to use
      shift and bitwise operations (expoent = 8 in order to sort 1 byet per iteration).
      This works a lot like the American Flag algorithm used to sort strings.</li>
<br>
  <li>Small preliminary check of the initial unsorted array to determine
      number of bytes to sort. Special useful in randomly shuffled arrays.</li>
<br>
  <li>The indexes of the buckets don't necessarily express the cumulative
      value of the occurrence of that given offset in the original array
      but rather the index at witch that ofsset starts in the array. This allows 
      for a much faster array traversal as it goes in ascending order and makes
      use of the sufix ++.</li>
   <br>
  <li>As there are only 4 iterations at max (for a 32 bit integer at least),
      instead of copying the whole helper array to the original at the end of 
      each iteration, the algorithm switches the purpose of these two arrays
      in order to reduce copying overhead (eventually correcting this at the
      end if it stops in a even number of bytes).</li>
<br>
  <li>The algorithms is the same even if the original array contains integers
      of different signs. The only thing we have to do at the end is starting
      from the negative numbers instead (in case of different signs). This task
      has a relatively small overhead.</li>
<br>
  <li>Neglecting the shift operation when sorting the first byte (equals >> 0).</li>
</ul>      

<h2><b>Benchmarks:</b></h2>
<p>All benchamrks were performed on a Intel Core-i7 6700, running Ubuntu 16.04.9.
<br>The program was compiled with GCC 5.4.0-6, with no compiler otimizations enabled.</p>
<p>The first table column shows the number of elements and the first row show the number of
bits that the maximun number in the array has. Mind that as the algorithm works byte
per byte, having 1 bit or 8 bits maximun is the same timewise.</p>

<h4><b>Same sign arrays:</b></h4>

|_Elements_      | 32 bits  | 24 bits  | 16 bits  | 8 bits   |
|:---------------|---------:|---------:|---------:|---------:|
|**1 Billion**   | 9.526307 | 8.196153 | 5.582001 | 3.656286 |
|**100 million** | 0.948754 | 0.812009 | 0.554831 | 0.360952 |
|**10 million**  | 0.094950 | 0.081082 | 0.055406 | 0.035727 |
|**1 million**   | 0.008333 | 0.006910 | 0.004460 | 0.002471 |

<h4><b>Different sign arrays:</b></h4>
<p>TODO</p>

<h2><b>Use and compilation:</b></h2>
<p>Provided you are in a Linux based OS, enter the following commands in your terminal:</p>
<p>Compile with: <code>gcc -Wall radix.c -o radix</code>
<br>Execute with: <code>./radix</code> </p>

<h2><b>More:</b></h2>

<h4><b>The code looks ugly!</b></h4>
<p>I couldn't agree more...
  <br>This is mainly because of the excessive use of macros and other
  small improvements that damages the code readability.
  <br>I would not have done this changes if the main objective wasn't performance</p>

<h4><b>Further improvement:</b></h4>
<p>This algorithm still has space for improving and if you want to optimize it further
  I can provide a cleaner version for clarity.</p>
<p>If you are kind enough, let me know what otimizations you were able to do.</p> 
