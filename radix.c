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
void int_radix_sort(register int vector[], register const unsigned int size);

/* Small functionality test */
int main(int argc, char * argv []) {

    /* Initial variable declaration */
    srand(time(NULL));
    clock_t t;
    int size = 100000000;
    int num_max = 2147483647;
    int num_min = 0;
    int i;

    /* User input */
    printf("Enter the number of elements: ");
    scanf("%d", &size);
    printf("Enter the minimun number: ");
    scanf("%d", &num_min);
    printf("Enter the maximun number: ");
    scanf("%d", &num_max);

    /* Check for inconsistensies and constrains */
    if(num_min > num_max) {
	int temp = num_min;
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
    t = clock();
    int_radix_sort(a, size);
    t = clock() - t;
    double time = (double)t / CLOCKS_PER_SEC;
  
    /* Print results */
    printf("\nRadix sort took %f seconds.\n[sorted %d numbers from %d to %d].\n",
	   time, size, num_min, num_max);

    if(array_sorted(a, size) != 0){
	printf("The array was sorted successfully!\n");
    } else {
    	printf("The array wasn't fully sorted. Please report this problem!\n");
    }

    free(a);
    
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
  Integer Radix LSD sort. Stable and out-of-place.
  ---Parameters---
  
  vector[] - Pointer to the orginal array of integers
  size     - Size of the array to sort
 
 ---List of optimizations implemented---
  For a list of all optimizations implemented check the github README.md
  over at https://github.com/AwardOfSky/Fast-Radix-Sort
 */
void int_radix_sort(register int vector[], register const unsigned int size) {

    /* Support for variable sized integers without overflow warnings */
    const int MAX_UINT__ = ((((1 << ((sizeof(int) << 3) - 2)) - 1) << 1) + 1);
    const int LAST_EXP__ = (sizeof(int) - 1) << 3;
    
    /* Define std preliminary, constrain and expression to check if all bytes are sorted */
#define PRELIMINARY__ 100
#define SET_MAX__ if(max < -exp) max = -exp;
#define MISSING_BITS__ exp < (sizeof(int) << 3) && (max >> exp) > 0
    /* Check for biggest integer in [a, b[ array segment */
#define LOOP_MAX__(a, b)				\
    for(s = &vector[a], k = &vector[b]; s < k; ++s) {	\
	if(*s > max || *s < exp) {			\
	    if(*s > max)  {				\
		max = *s;				\
	    } else{					\
		exp = *s;				\
	    }						\
	}						\
    }
    
    /* b = helper array pointer ; s, k and i = array iterators */
    /* exp = bits sorted, max = maximun range in array         */
    /* point = array of pointers to the helper array           */
    register int *b, *s, *k;
    register int exp = *vector;
    register int max = exp;
    int i, *point[0x100];
    int swap = 0;
    
    /* Set preliminary according to size */
    const int preliminary = (size > PRELIMINARY__) ? PRELIMINARY__ : (size >> 3);
    
    /* If we found a integer with more than 24 bits in preliminar, */
    /* will have to sort all bytes either way, so max = MAX_UINT__ */
    LOOP_MAX__(1, preliminary);
    SET_MAX__;
    if(max <= (MAX_UINT__ >> 7)) {	
	LOOP_MAX__(preliminary, size);
    }
    SET_MAX__;
    exp = 0;
    
    /* Helper array initialization */
    b = (int *)malloc(sizeof(int) * size);

    /* Core algorithm: for a specific byte, fill the buckets array, */
    /* rearrange the array and reset the initial array accordingly. */
#define SORT_BYTE__(vec, bb, shift)					\
    int bucket[0x100] = {0};						\
    register unsigned char *n = (unsigned char *)(vec) + (exp >> 3),*m; \
    for(m = (unsigned char *)(&vec[size & 0xFFFFFFFC]); n < m;) {	\
	++bucket[*n]; n += sizeof(int);					\
	++bucket[*n]; n += sizeof(int);					\
	++bucket[*n]; n += sizeof(int);					\
	++bucket[*n]; n += sizeof(int);					\
    }									\
    for(n = (unsigned char *)(&vec[size & 0xFFFFFFFC]) + (exp >> 3),	\
	    m = (unsigned char *)(&vec[size]); n < m;) {		\
	++bucket[*n]; n += sizeof(int);					\
    }									\
    s = bb;								\
    int next = 0;							\
    for(i = 0; i < 0x100; ++i) {					\
	if(bucket[i] == size) {						\
	    next = 1;							\
	    break;							\
	}								\
    }									\
    if(next) {								\
	exp += 8;							\
	continue;							\
    }									\
    if(exp == LAST_EXP__) {						\
	for(i = 128; i < 0x100; s += bucket[i++]) {			\
	    point[i] = s;						\
	}								\
	for(i = 0; i < 128; s += bucket[i++]) {				\
	    point[i] = s;						\
	}								\
    } else {								\
	for(i = 0; i < 0x100; s += bucket[i++]) {			\
	    point[i] = s;						\
	}								\
    }									\
    for(s = vec, k = &vec[size]; s < k; ++s) {				\
	*point[(*s shift) & 0xFF]++ = *s;				\
    }									\
    swap = 1 - swap;							\
    exp += 8;
    
    /* Sort each byte (if needed) */
    while(MISSING_BITS__) {
	if(exp) {
	    if(swap) {
		SORT_BYTE__(b, vector, >> exp);
	    } else {
		SORT_BYTE__(vector, b, >> exp);
	    }
	} else {
	    SORT_BYTE__(vector, b, );
	}
    }

    /* In case the array has both negative and positive integers, find the      */
    /* index of the first negative integer and put it in the start of the array */
    int *v = vector; /* No need to use registers here, the smaller their use, */
    int *y = b;      /* the better. */
    if(exp != (LAST_EXP__ + 8) && (((*v ^ v[size - 1]) < 0 && !swap) ||
    				   ((*y ^ y[size - 1]) < 0 && swap))) {
    	int offset = size - 1;
    	int tminusoff;
	
    	if(!swap)  {
    	    for(s = v, k = &v[size]; s < k && *s >= 0; ++s) { }
    	    offset = s - v;

    	    tminusoff = size - offset;
	    
    	    if(offset < tminusoff) {
    	    	memcpy(y, v, sizeof(int) * offset);
    	    	memcpy(v, v + offset, sizeof(int) * tminusoff);
    	    	memcpy(v + tminusoff, y, sizeof(int) * offset);
    	    } else {
    	    	memcpy(y, v + offset, sizeof(int) * tminusoff);
    	    	memmove(v + tminusoff, v, sizeof(int) * offset);
    	    	memcpy(v, y, sizeof(int) * tminusoff);
    	    }
    	} else {	    
    	    for(s = y, k = &y[size]; s < k && *s >= 0; ++s) { }
    	    offset = s - y;

    	    tminusoff = size - offset;

    	    memcpy(v, y + offset, sizeof(int) * tminusoff);
    	    memcpy(v + tminusoff, y, sizeof(int) * (size - tminusoff));
    	}

    } else if(swap) {
    	memcpy(v, y, sizeof(int) * size);
    }

    /* Free helper array */
    free(b);
    
    /* Undefine function scoped macros for eventual later use */
#undef PRELIMINARY__
#undef SET_MAX__
#undef MISSING_BITS__
#undef LOOP_MAX__
#undef SORT_BYTE__
    
}
