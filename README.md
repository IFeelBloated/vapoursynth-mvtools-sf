# MVTools-Float
Single Precision MVTools

Requires libfftw3-3.dll to be in the search path. http://www.fftw.org/install/windows.html

## Compilation

### Linux

```
$ meson build
$ ninja -C build
```

### Manual

```
g++ -shared -std=c++20 -lstdc++ -static -Ofast -Wno-subobject-linkage -o Filter.dll EntryPoint.cxx vapoursynth.lib libfftw3-3.lib
```
