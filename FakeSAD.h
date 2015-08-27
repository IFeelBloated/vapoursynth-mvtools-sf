#ifndef FAKESAD_H
#define FAKESAD_H

union fakestuff {
	int fake;
	float real;
};

inline static int FakeInt(float a) {
	fakestuff tmp;
	tmp.real = a;
	return tmp.fake;
}

inline static float Back2FLT(int a) {
	fakestuff tmp;
	tmp.fake = a;
	return tmp.real;
}

#endif
