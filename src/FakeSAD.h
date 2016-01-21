#ifndef FAKESAD_H
#define FAKESAD_H

union fakestuff {
	int fake;
	float real;
};

static inline int FakeInt(float a) {
	fakestuff tmp;
	tmp.real = a;
	return tmp.fake;
}

static inline float Back2FLT(int a) {
	fakestuff tmp;
	tmp.fake = a;
	return tmp.real;
}

#endif
