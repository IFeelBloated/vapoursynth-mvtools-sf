project('vapoursynth-mvtools-sf', 'cpp', 
    default_options: ['buildtype=release', 'b_ndebug=if-release', 'cpp_std=c++2a'],
    version : '10'
)

if not get_option('buildtype').contains('debug')
    add_project_arguments('-Ofast', language : 'cpp')
endif

if host_machine.cpu_family().startswith('x86')
  add_project_arguments('-mfpmath=sse', '-msse2', language : 'cpp')
endif

vs = dependency('vapoursynth')
vsfs = dependency('vsfilterscript')
fftw = dependency('fftw3')

src = ['src/EntryPoint.cxx']

shared_module('vapoursynth-mvtools-sf', src,
    dependencies : [vs, vsfs, fftw],
    gnu_symbol_visibility : 'hidden',
    install_dir : join_paths(vs.get_pkgconfig_variable('libdir'), 'vapoursynth'),
    install : true
)
