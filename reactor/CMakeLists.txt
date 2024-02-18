cmake_minimum_required(VERSION 3.10)
project(reactor)

aux_source_directory(./src/ SRC)# 搜索源文件
set(LIBRARY_OUTPUT_PATH ${LIBPATH}) # 库生成的路径 LIBPATH

# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include) # 头文件路径 HEADPATH
include_directories(./include) # 头文件路径 HEADPATH

add_library(reactor STATIC ${SRC}) # 静态库
# add_library(reactor SHARED ${SRC}) # 动态库