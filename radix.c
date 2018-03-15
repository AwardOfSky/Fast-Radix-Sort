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

#define MAX_INT 2147483647
#define MAX_INT_24 16777215

int array_sorted(int vector[], int size);
void int_radix_sort(register int vector[], register const int size, int same_sign);

/* Small functionality test */
int main(int argc, char * argv []) {

    /* Initial variable declaration */
    srand(time(NULL));
    clock_t start, end;
    int size = 100000000;
    int num_max = MAX_INT;
    int num_min = 0;
    int i, temp, same_sign;

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

    /* Set the sign flag (this is optional in real case scenarios) */
    if(num_min < 0) {
	same_sign = 0;
    } else {
	same_sign = 1;
    }

    /* Time sort function execution */
    start = clock();
    int_radix_sort(a, size, same_sign);
    end = clock();

    /* Print results */
    printf("\nRadix sort took %.6f seconds.\n[sorted %d numbers from %d to %d].\n",
	   (double)(end - start) / CLOCKS_PER_SEC, size, num_min, num_max);

    if(array_sorted(a, size) != 0){
	printf("The array was sorted successfully!\n");
    } else {
    	printf("The array wasn't fully sorted. Please report this problem!\n");
    }

    for(i = 0; i < size; i++) {
	printf("%d\n", a[i]);
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
  samesign - Wether the orignal array contains both positive and negative integers.
             Set this parameter to 0 if not (or not sure), and to 1 otherwise.
	     This parameter is mostly here to squeeze some performance (same sign
	     arrays happen frequently in real case scenarios).

  ---List of optimizations implemented---

  1 - Use of powers of 2 for the expoents and bucket size in order to use
      shift and bitwise operations (expoent = 8 in order to sort 1 byte per iteration).
      This works a lot like the American Flag algorithm used to sort strings.

  2 - Small preliminary check of the initial unsorted array to determine
      number of bytes to sort. Special useful in randomly shuffled arrays.

  3 - The indexes of the buckets don't necessarily express the cumulative
      value of the occurrence of that given offset in the original array
      but rather the index at witch that ofsset starts in the array. This allows 
      for a much faster array traversal as it goes in ascending order and makes
      use of the sufix ++.
   
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
void int_radix_sort(register int vector[], register const int size, int same_sign) {

    /* Define standard preliminar, abs and expression to check if all bytes are sorted */
#define PRELIMINARY__ 100
#define ABS__(x) (((x) < 0) ? -(x) : (x))
#define MISSING_BITS__ exp < 32 && (max >> exp) > 0
    /* Define array segment to search max number */
#define CHECK_MAX__(a, b)			\
    if(same_sign && *vector >= 0) {		\
	LOOP_MAX__(>, *s, a, b);		\
    } else if(same_sign && *vector < 0) {	\
	LOOP_MAX__(<, *s, a, b);		\
    } else {					\
	LOOP_MAX__(>, ABS__(*s), a, b);		\
    }
    /* Check for biggest integer in [a, b] array segment */
#define LOOP_MAX__(S, V, a, b)				\
    for(s = &vector[a], k = &vector[b]; s < k; ++s) {	\
	if((V) S max) {					\
	    max = (V);					\
	}						\
    }

    /* b = pointer to helper array; s and k = array iterators */
    /* exp = bits sorted, max = maximun number in array       */
    register int *b, *s, *k;
    register int exp = 0;
    register int max = *vector;
    int preliminary;
    
    /* Set preliminar according to size */
    if(size > PRELIMINARY__) {
	preliminary = PRELIMINARY__;
    } else {
	preliminary = size >> 3;
    }

    /* If we found a integer with more than 24 bits in preliminar, */
    /* will have to sort all 4 bytes either way, so max = MAX_INT  */
    CHECK_MAX__(0, preliminary);
    if(ABS__(max) > MAX_INT_24) {
    	max = MAX_INT;
    } else {
	CHECK_MAX__(preliminary, size);
    }

    /* Helper array declaration */
    b = (int *)malloc(sizeof(int) * size);

    /* Check if last byte sorted was odd */
#define BYTE_IS_ODD__ ((exp >> 3) & 1)

    /* Core algorithm: for a specific byte, fill the buckets array, */
    /* rearrange the array and reset the initial array accordingly. */
#define SORT_BYTE__(bu, vet, bb, xp)			\
    int bu[256] = {0};					\
    for(s = vet, k = &vet[size]; s < k; ++s) {		\
	bu[(*s xp) & 0xFF]++;				\
    }							\
    for(s = &bu[1], k = &bu[256]; s < k; ++s) {		\
	*s += *(s - 1);					\
    }							\
    memmove(bu + 1, bu, sizeof(int) * 255);		\
    *bu = 0;						\
    for(s = vet, k = &vet[size]; s < k; ++s) {		\
	bb[bu[(*s xp) & 0xFF]++] = *s;			\
    }							\
    exp += 8;

    /* Sort each byte (if needed) */
    SORT_BYTE__(bucket, vector, b, );
    if(MISSING_BITS__) {
	SORT_BYTE__(bucket1, b, vector, >> exp);
	if(MISSING_BITS__) {
	    SORT_BYTE__(bucket2, vector, b, >> exp);
	    if(MISSING_BITS__) {
		SORT_BYTE__(bucket3, b, vector, >> exp);
	    }
	}
    }

    /* If last byte sorted was even, the sorted array will be the helper, */
    /* Therefore we will have to put it in the original array             */
    if(BYTE_IS_ODD__) {
	memcpy(vector, b, sizeof(int) * size);
    }

    /* In case the array has both negative and positive integers, find the      */
    /* index of the first negative integer and put it in the start of the array */
    if(!same_sign) {
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
#undef MISSING_BITS__
#undef ABS__
#undef CHECK_MAX__
#undef LOOP_MAX__
#undef SORT_BYTE__
#undef BYTE_IS_EVEN__

}
