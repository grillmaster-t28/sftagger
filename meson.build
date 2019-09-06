project('sftagger'
  , 'cpp'
  , default_options : ['warning_level=3', 'cpp_std=c++17', 'optimization=3', 'werror=false']
)

add_project_arguments(['-lstdc++fs'], language : 'cpp')

src_inc = include_directories('./src/')
local_inc = include_directories('/usr/local/include/')

sqlite3_dep = dependency('sqlite3', required : true)

src = [
    './src/main.cpp'
  , './src/tools/homeDirectory.cpp'
  , './src/tools/argumentsToVector.cpp'
  , './src/core/create.cpp'
]

executable('sftagger'
           , sources : src
           , dependencies : [sqlite3_dep]
           , include_directories : [src_inc, local_inc]
)


