# Fast-Radix-Sort
An optimized implementation of the Radix LSD sorting algorithm for integers, written in C.

<h2><b>Algorithm:</b></h2>
<p>Radix Sort is a non-comparative sorting algorithm able to sort a list of N integers in <b>O(kn) time</b> complexity.
<br>The presented program showcases an implementation of said algorithm written in <b>ANSI C</b> and optimized for speed.</p>
<p>The function sorts 32-bit <b>integers (signed or unsigned)</b>, one byte at a time (a bit like the American Flag sorting,
<br>but starting with the Least Significant Digit/Byte).</p>
<p>Radix sort is a <b>stable</b> sorting, meaning every two numbers with the same value appear in the same order in the
   sorted output as they appear in the input.</p>
<p>This implementation follows an <b>out-of-place</b> approach, meaning it uses an helper array in order to sort
  the original array.</p>
<p>The program supports all main C compilers (GCC, Clang, MCVS, ...) and architectures.
<p>For more information about the algorithm itself, check the
<a target="_blank" href="https://en.wikipedia.org/wiki/Radix_sort">Wikipedia Page</a>.</p>

<h2><b>Optimizations:</b></h2>

<p>The sorting function was made without the help of compiler optimizations in mind.
<br>Therefore, it uses some microtimizations like macros and registers to speed some things up.
<br>Be mindful that compilers like GCC don't provide full-fledged optimizations unless told so.
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
  <li>The indexes of the buckets express the amount of elements of that respective
      index in the original array. There is also a array of pointers so that
      each pointer has the adress in the helper array where the given offset
      should start.</li>
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
      has a relatively small overhead as it is done with the help of memcpy and
      memmove.<p>This only applies if we're not sorting all the bytes (see last point)!</li>
<br>
  <li>Neglecting the shift operation when sorting the first byte (equals >> 0).</li>
<br>
  <li>If we have to sort all bytes either way, we can deal with negative numbers by 
      splitting the for loop that assign the pointers to the helper array in half.
      This works because the last bit is the sign bit, thus in the last byte the negative
      numbers will correspond to the buckets with an index higher that 127.
      Although a small improvement this allows for sorting negative and positive integers
      in the same time complexity (if sorting all bytes), bringing down the time for the worst
      case scenario in a random shuffled array.</li>
</ul>      

<p>Mind that as the algorithm works byte per byte, having 1 bit or 8 bits maximun is the same timewise.</p>

<h2><b>Use and compilation:</b></h2>
<p>Provided you are in a Linux based OS, enter the following commands in your terminal:</p>
<p>Compile with: <code>gcc -Wall radix.c -o radix</code>
<br>Execute with: <code>./radix</code> </p>

<h2><b>More:</b></h2>

<h4><b>The code looks ugly!</b></h4>
<p>I couldn't agree more...
  <br>This is mainly because of the excessive use of microptimizations, macros and other
  small improvements that damages the code readability.
  <br>Keep in mind the main objective is performance.</p>

<h4><b>Further improvement:</b></h4>
<p>This algorithm still has space for improving and if you want to optimize it further
  I can provide a cleaner version for clarity.
<br>If you are kind enough, let me know what optimizations you were able to do.</p>
