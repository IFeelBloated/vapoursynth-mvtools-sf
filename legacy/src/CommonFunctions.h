#ifndef __COMMON_F__
#define __COMMON_F__

static inline int32_t satz(int32_t a) {
	return ~(a >> (sizeof(int32_t) * 8 - 1)) & a;
}

static inline int32_t imax(int32_t a, int32_t b) {
	return a + satz(b - a);
}

static inline int32_t imin(int32_t a, int32_t b) {
	return a - satz(a - b);
}

static inline int32_t ilog2(int32_t i) {
	int32_t result = 0;
	while (i > 1) { i /= 2; result++; }
	return result;
}

static inline int32_t iexp2(int32_t i) {
	return 1 << satz(i);
}

static inline int64_t gcd(int64_t u, int64_t v) {
	int32_t shift;
	if (u == 0 || v == 0)
		return u | v;
	for (shift = 0; ((u | v) & 1) == 0; ++shift) {
		u >>= 1;
		v >>= 1;
	}
	while ((u & 1) == 0)
		u >>= 1;
	do {
		while ((v & 1) == 0)
			v >>= 1;
		if (u < v) {
			v -= u;
		}
		else {
			int64_t diff = u - v;
			u = v;
			v = diff;
		}
		v >>= 1;
	} while (v != 0);
	return u << shift;
}

#endif
