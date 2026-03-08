project(
    'nix-owo',
    'cpp',
    version: '0.1.0',
    default_options : ['cpp_std=c++23']
)

nix_util_dep = dependency('nix-util', method: 'pkg-config')

sources = files(
    'main.cpp',
)

executable('nix-hash', sources, dependencies : [
    nix_util_dep
])
