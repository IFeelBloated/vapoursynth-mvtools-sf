AM_CXXFLAGS = -O2 -std=c++14 -msse2 -mfpmath=sse $(STACKREALIGN) -Wall -Wextra -Wno-unused-parameter -Wshadow

AM_CPPFLAGS = $(VapourSynth_CFLAGS) $(FFTW3_CFLAGS)

lib_LTLIBRARIES = libmvtoolssf.la

libmvtoolssf_la_SOURCES =	 \
						src/EntryPoint.cpp \
						src/FakeGroupOfPlanes.cpp \
						src/GroupOfPlanes.cpp \
						src/MVAnalyze.cpp \
						src/MVBlockFPS.cpp \
						src/MVClip.cpp \
						src/MVCompensate.cpp \
						src/MVDegrains.cpp \
						src/MVFinest.cpp \
						src/MVFlow.cpp \
						src/MVFlowBlur.cpp \
						src/MVFlowFPS.cpp \
						src/MVFlowInter.cpp \
						src/MVFrame.cpp \
						src/MVMask.cpp \
						src/MVRecalculate.cpp \
						src/MVSCDetection.cpp \
						src/MVSuper.cpp \
						src/Overlap.cpp \
						src/PlaneOfBlocks.cpp

libmvtoolssf_la_LDFLAGS = -no-undefined -avoid-version $(PLUGINLDFLAGS)

libmvtoolssf_la_LIBADD = $(FFTW3_LIBS)
