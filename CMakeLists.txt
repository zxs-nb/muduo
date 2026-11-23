cmake_minimum_required(VERSION 3.16)
project(mymuduo LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARDD_REQUIRED ON)

# 定义参与编译的源代码文件
aux_source_directory(. SRC_LIST)

# 编译生成动态库mymuduo
add_library(mymuduo SHARED ${SRC_LIST})

# 设置共享库输出目录
set_target_properties(mymuduo PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
)

# set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)