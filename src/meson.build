project(
    'nix-owo',
    'cpp',
    version: '0.1.0',
    default_options : ['cpp_std=c++23']
)

nix_util_dep = dependency('nix-util', method: 'pkg-config')

hash_sources = files(
    'main.cpp',
)

owo_sources = files(
    'owo.cpp'
)

executable('nix-hash', hash_sources, dependencies : [
    nix_util_dep
])

executable('nix-owo', owo_sources, dependencies : [
    nix_util_dep
])
