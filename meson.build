project('sftagger'
  , 'cpp'
  , default_options : ['warning_level=3', 'cpp_std=c++17', 'optimization=3', 'werror=false']
)

add_project_arguments(['-lstdc++fs'], language : 'cpp')

src_inc = include_directories('./src/')

src = [
    './src/main.cpp'
  , './src/tools/homeDirectory.cpp'
  , './src/tools/argumentsToVector.cpp'
]

executable('sftagger', sources : src, include_directories : src_inc)


