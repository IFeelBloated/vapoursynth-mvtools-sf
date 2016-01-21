#ifndef __COMMON_F__
#define __COMMON_F__

static inline int satz(int a) {
	return ~(a >> (sizeof(int) * 8 - 1)) & a;
}

static inline int imax(int a, int b) {
	return a + satz(b - a);
}

static inline int imin(int a, int b) {
	return a - satz(a - b);
}

static inline int ilog2(int i) {
	int result = 0;
	while (i > 1) { i /= 2; result++; }
	return result;
}

static inline int iexp2(int i) {
	return 1 << satz(i);
}

static inline int64_t gcd(int64_t u, int64_t v) {
	int shift;
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
