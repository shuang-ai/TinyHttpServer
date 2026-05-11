#!/bin/bash

# 定义颜色输出，方便查看状态
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}开始构建项目...${NC}"

# 1. 创建 build 目录用于存放中间编译文件 (保持源码目录整洁)
# 如果不想用 out-of-source build，也可以直接在当前目录 cmake .，但推荐以下方式
if [ ! -d "build" ]; then
    mkdir build
fi

# 2. 进入 build 目录进行 cmake 配置和 make 编译
cd build

# 生成 Makefile
# .. 表示 CMakeLists.txt 在上一级目录
cmake .. 

# 检查 cmake 是否成功
if [ $? -ne 0 ]; then
    echo -e "${RED}CMake 配置失败!${NC}"
    exit 1
fi

# 编译
make -j4

# 检查 make 是否成功
if [ $? -ne 0 ]; then
    echo -e "${RED}编译失败!${NC}"
    cd ..
    exit 1
fi

# 回到项目根目录
cd ..

# 3. 提示用户
echo -e "${GREEN}构建成功!${NC}"
echo "可执行文件位于: ./bin/thttpd.out"
echo "请手动执行: ./bin/thttpd.out [端口号]"