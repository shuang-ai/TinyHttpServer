#!/bin/bash

# 获取脚本文件所在的真实目录（关键！）
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 项目根目录就是 SCRIPT_DIR（因为 run.sh 在根目录）
PROJECT_DIR="$SCRIPT_DIR"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}开始构建项目...${NC}"

# 使用绝对路径或相对于 PROJECT_DIR 的路径
cd "$PROJECT_DIR"

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# 使用 PROJECT_DIR 而不是 ..
cmake "$PROJECT_DIR"

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake 配置失败!${NC}"
    exit 1
fi

make -j4

if [ $? -ne 0 ]; then
    echo -e "${RED}编译失败!${NC}"
    cd "$PROJECT_DIR"
    exit 1
fi

cd "$PROJECT_DIR"

echo -e "${GREEN}构建成功!${NC}"
echo "可执行文件位于: ./bin/thttpd.out"http://localhost:8888/notexist.html