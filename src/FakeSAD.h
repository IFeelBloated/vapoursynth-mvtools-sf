#ifndef FAKESAD_H
#define FAKESAD_H

union fakestuff {
	int32_t fake;
	float real;
};

static inline int32_t FakeInt(float a) {
	fakestuff tmp;
	tmp.real = a;
	return tmp.fake;
}

static inline float Back2FLT(int32_t a) {
	fakestuff tmp;
	tmp.fake = a;
	return tmp.real;
}

#endif
