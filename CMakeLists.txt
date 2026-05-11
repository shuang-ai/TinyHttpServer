# 规定最小版本
cmake_minimum_required(VERSION 3.0)

# 项目名称
project(thttpd)

# 设置可执行文件的输出目录为项目根目录下的 bin 文件夹
# CMAKE_BINARY_DIR 指的是 cmake 运行时的构建目录，这里我们强制指定到源码同级目录的 bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 添加可执行文件
# 注意：生成的文件名依然是 thttpd.out，但它会出现在 ./bin/thttpd.out
add_executable(thttpd.out main.cpp http.cpp custom_handle.cpp)

# 链接线程支持库
target_link_libraries(thttpd.out pthread)