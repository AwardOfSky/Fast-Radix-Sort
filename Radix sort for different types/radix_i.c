
void radix_i(register int vector[], register const int size) {

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
