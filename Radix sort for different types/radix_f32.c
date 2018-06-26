
void radix_f32(float array[], register const int32_t size) {
    
    /* Floats are 4 bytes across all plataforms */
    register int32_t *vector = (int32_t *)array;
    const int32_t LAST_EXP__ = 24;
    
    /* b = helper array pointer ; s, k and i = array iterators */
    /* exp = bits sorted, max = maximun range in array         */
    /* point = array of pointers to the helper array           */
    register int32_t *b, *s, *k;
    register int32_t exp = 0;
    int32_t *point[0x100];
    int32_t i, swap = 0; /* see when vector and helper are swaped */

    /* Helper array initialization */
    b = (int32_t *)malloc(size << 2);

    /* pre-compute buckets as we are sorting all bytes either way */
    int32_t bucket0[0x100 * sizeof(int32_t)] = {0};
    int32_t *bucket1 = bucket0 + 0x100;
    int32_t *bucket2 = bucket1 + 0x100;
    int32_t *bucket3 = bucket2 + 0x100;
    
    register unsigned char *n, *m = (unsigned char *)(&vector[size]);
    for(n = (unsigned char *)(vector); n < m;) {
	++bucket0[*n++];
	++bucket1[*n++];
	++bucket2[*n++];
	++bucket3[*n++];
    }
	
    /* Core algorithm: for a specific byte, fill the buckets array, */
    /* rearrange the array and reset the initial array accordingly. */
#define MISSING_BITS__ exp < 32
#define SORT_BYTE__(vec, bb, shift)					\
    s = bb;								\
    int32_t *buck = bucket0 + (exp << 5);				\
    int32_t next = 0;							\
    for(i = 0; i < 0x100; ++i) {					\
	if(buck[i] == size) {						\
	    next = 1;							\
	    break;							\
	}								\
    }									\
    if(next) {								\
	exp += 8;							\
	continue;							\
    }									\
    if(exp == LAST_EXP__) {						\
	for(i = 0xFF; i >= 128; s += buck[--i]) {			\
	    point[i] = s;						\
	}								\
	for(i = 0; i < 128; s += buck[i++]) {				\
	    point[i] = s;						\
	}								\
    } else {								\
	for(i = 0; i < 0x100; s += buck[i++]) {				\
	    point[i] = s;						\
	}								\
    }									\
    if(exp != LAST_EXP__) {						\
	for(s = vec, k = &vec[size]; s < k; ++s) {			\
	    *point[(*s shift) & 0xFF]++ = *s;				\
	}								\
    } else {								\
	for(s = vec, k = &vec[size]; s < k; ++s) {			\
	    register int32_t index = (*s shift) & 0xFF;			\
	    if(index >= 128) {						\
		*--point[index] = *s;					\
	    } else {							\
		*point[index]++ = *s;					\
	    }								\
	}								\
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

    /* If we skipped ahead bytes and the sorted array is in the healper, */
    /* We have to copy it over to the original array*/
    if(swap) {
	memcpy(vector, b, size << 2);
    }
    
    /* Free helper array */
    free(b);
    
    /* Undefine function scoped macros for eventual later use */
#undef MISSING_BITS__
#undef SORT_BYTE__
    
}
