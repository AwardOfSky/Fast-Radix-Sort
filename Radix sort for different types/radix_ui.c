
void radix_ui(register unsigned int vector[], register const unsigned int size) {

    /* Support for variable sized integers without overflow warnings */
    const int MAX_UINT__ = ((((1 << ((sizeof(unsigned int) << 3) - 2)) - 1) << 1) + 1);
    const int LAST_EXP__ = (sizeof(unsigned int) - 1) << 3;
    
    /* Define std preliminary, constrain and expression to check if all bytes are sorted */
#define PRELIMINARY__ 100
#define MISSING_BITS__ exp < (sizeof(unsigned int) << 3) && (max >> exp) > 0
    /* Check for biggest integer in [a, b[ array segment */
#define LOOP_MAX__(a, b)				\
    for(s = &vector[a], k = &vector[b]; s < k; ++s) {	\
	if(*s > max)  {					\
	    max = *s;					\
	}						\
    }
    
    /* b = helper array pointer ; s, k and i = array iterators */
    /* exp = bits sorted, max = maximun range in array         */
    /* point = array of pointers to the helper array           */
    register unsigned int *b, *s, *k;
    register unsigned int exp = 0;
    register unsigned int max = exp;
    unsigned int i, *point[0x100];
    int swap = 0;
    
    /* Set preliminary according to size */
    const unsigned int preliminary = (size > PRELIMINARY__) ? PRELIMINARY__ : (size >> 3);
    
    /* If we found a integer with more than 24 bits in preliminar, */
    /* will have to sort all bytes either way, so max = MAX_UINT__ */
    LOOP_MAX__(1, preliminary);
    if(max <= (MAX_UINT__ >> 7)) {	
	LOOP_MAX__(preliminary, size);
    }
    
    /* Helper array initialization */
    b = (unsigned int *)malloc(sizeof(unsigned int) * size);
    
    /* Core algorithm: for a specific byte, fill the buckets array, */
    /* rearrange the array and reset the initial array accordingly. */
#define SORT_BYTE__(vec, bb, shift)					\
    unsigned int bucket[0x100] = {0};					\
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
    for(i = 0; i < 0x100; s += bucket[i++]) {				\
	point[i] = s;							\
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

    if(swap) {
	memcpy(vector, b, sizeof(unsigned int) * size);
    }
    
    /* Free helper array */
    free(b);
    
    /* Undefine function scoped macros for eventual later use */
#undef PRELIMINARY__
#undef MISSING_BITS__
#undef LOOP_MAX__
#undef SORT_BYTE__
    
}
