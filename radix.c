/* 
     This program was tested with GCC 5.4.0 on Ubuntu 16.04
  and has backwards compatibility with C89 (flag -std=c89).
  No compiler specific directives or architecture intrinsics
  were used.

  Compile with: gcc -Wall radix.c -o radix 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int array_sorted(int vector[], int size);
void int_radix_sort(register int vector[], register const int size);

/* Small functionality test */
int main(int argc, char * argv []) {

    /* Initial variable declaration */
    srand(time(NULL));
    clock_t start, end;
    int size = 100000000;
    int num_max = 2147483647;
    int num_min = 0;
    int i, temp;

    /* User input */
    printf("Enter the number of elements: ");
    scanf("%d", &size);
    printf("Enter the minimun number: ");
    scanf("%d", &num_min);
    printf("Enter the maximun number: ");
    scanf("%d", &num_max);

    /* Check inconsistensies and constrains */
    if(num_min > num_max) {
	temp = num_min;
	num_min = num_max;
	num_max = temp;
    }
    if(size < 0) {
	size = -size;
    }

    /* Allocate and randomly shuffle the array */
    int *a = (int *)malloc(sizeof(int) * size);
    for(i = 0; i < size; ++i) {
	a[i] = (rand() % (num_max - num_min)) + num_min;
    }

    /* Time sort function execution */
    start = clock();
    int_radix_sort(a, size);
    end = clock();

    /* Print results */
    printf("\nRadix sort took %.6f seconds.\n[sorted %d numbers from %d to %d].\n",
	   (double)(end - start) / CLOCKS_PER_SEC, size, num_min, num_max);

    if(array_sorted(a, size) != 0){
	printf("The array was sorted successfully!\n");
    } else {
    	printf("The array wasn't fully sorted. Please report this problem!\n");
    }

    return 0;
}

/* Sanity check for unsorted elements */
int array_sorted(int vector[], int size) {
    int i, flag = 1;
    for(i = 1; i < size && flag; ++i) {
	if(vector[i] < vector[i - 1]) {
	    flag = 0;
	}
    }
    return flag;
}

/*
  This function will perform an optimized version of 
  Radix LSD sort (not in place).

  ---Parameters---
  
  vector[] - Pointer to the orginal array of integers
  size     - Size of the array to sort
 
 ---List of optimizations implemented---

  1 - Use of powers of 2 for the expoents and bucket size in order to use
      shift and bitwise operations (expoent = 8 in order to sort 1 byte per iteration).
      This works a lot like the American Flag algorithm used to sort strings.

  2 - Small preliminary check of the initial unsorted array to determine
      number of bytes to sort. Special useful in randomly shuffled arrays.

  3 - The indexes of the buckets express the amount of elements of that respective
      index in the original array. There is also a array of pointers so that
      each pointer has the adress in the helper array where the given offset
      should start.
   
  4 - As there are only 4 iterations at max (for a 32 bit integer at least),
      instead of copying the whole helper array to the original at the end of 
      each iteration, the algorithm switches the purpose of these two arrays
      in order to reduce copying overhead (eventually correcting this at the
      end if it stops in a even number of bytes).

  5 - The algorithms is the same even if the original array contains integers
      of different signs. The only thing we have to do at the end is starting
      from the negative numbers instead (in case of different signs). This task
      has a relatively small overhead.

  6 - Neglecting the shift operation when sorting the first byte (equals >> 0).
      
  Adding to the main structural optimizations of the algorithm, there are a
  number of microoptimizations to further improve performance. These include
  the use (and abuse!) of macros, traversing the array with pointers instead of
  indexes, use of standard C functions like memcpy to efficiently copy arrays
  segments, prefix increments rather than sufix (if possible), registers, etc

 */
void int_radix_sort(register int vector[], register const int size) {

    /* Support for variable sized integers without overflow warnings */
    const int MAX_UINT__ = ((((1 << ((sizeof(int) << 3) - 2)) - 1) << 1) + 1);
    
    /* Define standard preliminar, constrain and expression to check if all bytes are sorted */
#define PRELIMINARY__ 100
#define CONSTRAIN__(a, b, n) ((n) < (a) ? (a) : ((n) > (b) ? (b) : (n)))
#define MISSING_BITS__ exp < (sizeof(int) << 3) && (max >> exp) > 0
    /* Check for biggest integer in [a, b[ array segment */
#define LOOP_MAX__(a, b)				\
    for(s = &vector[a], k = &vector[b]; s < k; ++s) {	\
	if(*s > max) {					\
	    max = *s;					\
	}						\
	if(*s < exp) {					\
	    exp = *s;					\
	}						\
    }

    /* b = helper array pointer ; s and k = array iterators */
    /* exp = bits sorted, max = maximun number in array     */
    /* point = array of pointers to the helper array        */
    register int *b, *s, *k;
    register int exp = *vector;
    register int max = exp;
    int preliminary, i;
    int *point[0x100];
    
    /* Set preliminary according to size */
    if(size > PRELIMINARY__) {
	preliminary = PRELIMINARY__;
    } else {
	preliminary = size >> 3;
    }

    /* If we found a integer with more than 24 bits in preliminar, */
    /* will have to sort all bytes either way, so max = MAX_UINT__ */
    LOOP_MAX__(1, preliminary);
    if(CONSTRAIN__(0, MAX_UINT__, (unsigned int)(max - exp)) > (MAX_UINT__ >> 7)) {
    	max = MAX_UINT__;
    } else {
	LOOP_MAX__(preliminary, size);
    }
    max = CONSTRAIN__(0, MAX_UINT__, (unsigned int)(max - exp));
    exp = 0;
    
    /* Helper array initialization */
    b = (int *)malloc(sizeof(int) * size);
    
    /* Core algorithm: for a specific byte, fill the buckets array, */
    /* rearrange the array and reset the initial array accordingly. */
#define BYTE_IS_ODD__ ((exp >> 3) & 1)
#define SORT_BYTE__(vec, bb, shift)					\
    int bucket[0x100] = {0};						\
    for(s = vec, k = &vec[size]; s < k; ++s) {				\
	bucket[(*s shift) & 0xFF]++;					\
    }									\
    s = bb;								\
    for(i = 0; i < 0x100; s += bucket[i++]) {				\
	point[i] = s;							\
    }									\
    for(s = vec, k = &vec[size]; s < k; ++s) {				\
	*point[(*s shift) & 0xFF]++ = *s;				\
    }									\
    exp += 8;

    /* Sort each byte (if needed) */
    while(MISSING_BITS__) {
	if(exp) {
	    if(BYTE_IS_ODD__) {
		SORT_BYTE__(b, vector, >> exp);
	    } else {
		SORT_BYTE__(vector, b, >> exp);
	    }
	} else {
	    SORT_BYTE__(vector, b, );
	}
    }
	
    /* If last byte sorted was odd, the sorted array will be the helper, */
    /* Therefore we will have to put it in the original array            */
    if(BYTE_IS_ODD__) {
	memcpy(vector, b, sizeof(int) * size);
    }
    
    /* In case the array has both negative and positive integers, find the      */
    /* index of the first negative integer and put it in the start of the array */
    if((*vector ^ vector[size - 1]) < 0) {
	if(!BYTE_IS_ODD__) {
	    memcpy(b, vector, sizeof(int) * size);
	}

	int offset = size - 1;
    	int tminusoff;
	for(s = b, k = &b[size]; s < k && *s >= 0; ++s) { }
	offset = s - b;
	tminusoff = size - offset;

	memcpy(vector, b + offset, sizeof(int) * tminusoff);
	memcpy(vector + tminusoff, b, sizeof(int) * (size - tminusoff));	
    }

    /* Free helper array */
    free(b);
    
    /* Undefine function scoped macros for eventual later use */
#undef PRELIMINARY__
#undef CONSTRAIN__
#undef MISSING_BITS__
#undef LOOP_MAX__
#undef SORT_BYTE__
#undef BYTE_IS_ODD__
    
}
