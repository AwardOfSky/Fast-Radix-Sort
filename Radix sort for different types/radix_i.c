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
    register unsigned char *n, *m = (unsigned char *)(&vec[size]);	\
    for(n = (unsigned char *)(vec) + (exp >> 3);			\
	n < m; n += sizeof(int)) {					\
	++bucket[*n];							\
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
    if(exp != (LAST_EXP__ + 8) && (((*vector ^ vector[size - 1]) < 0 && !swap) ||
				   ((*b ^ b[size - 1]) < 0 && swap))) {
	int offset = size - 1;
    	int tminusoff;

	if(!swap)  {
	    for(s = vector, k = &vector[size]; s < k && *s >= 0; ++s) { }
	    offset = s - vector;

	    tminusoff = size - offset;

	    if(offset < tminusoff) {
		memcpy(b, vector, sizeof(int) * offset);
		memcpy(vector, vector + offset, sizeof(int) * tminusoff);
		memcpy(vector + tminusoff, b, sizeof(int) * offset);
	    } else {
		memcpy(b, vector + offset, sizeof(int) * tminusoff);
		memmove(vector + tminusoff, vector, sizeof(int) * offset);
		memcpy(vector, b, sizeof(int) * tminusoff);
	    }
	} else {
	    for(s = b, k = &b[size]; s < k && *s >= 0; ++s) { }
	    offset = s - b;

	    tminusoff = size - offset;

	    memcpy(vector, b + offset, sizeof(int) * tminusoff);
	    memcpy(vector + tminusoff, b, sizeof(int) * (size - tminusoff));	
	}

    } else if(swap) {
	memcpy(vector, b, sizeof(int) * size);
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
