# Created By 曹顺 
cmake_minimum_required(VERSION 3.13)
project(parse_e_file)

set(CMAKE_CXX_STANDARD 14)

set(DIRS .)

include_directories(${DIRS})

foreach (DIR ${DIRS})
    aux_source_directory(${DIR} FILES)
endforeach ()

add_executable(parse_e_file ${FILES})
