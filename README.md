# MVTools-Float
Single Precision MVTools

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
