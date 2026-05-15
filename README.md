# TinyHttpServer - 轻量级 C++ HTTP 服务器

## 项目简介

这是一个基于 C++ 编写的轻量级 HTTP 服务器，支持处理 GET 和 POST 请求，可处理静态文件服务和简单的动态业务逻辑。

## 目录结构
http/
├── wwwroot/ # Web 根目录（静态文件）
│ ├── index.html # 默认首页
│ ├── 404.html # 404 错误页面
│ ├── login.html # 登录页面（示例）
│ └── post.html # POST 请求示例页面
├── custom_handle.cpp # 自定义请求处理实现
├── custom_handle.h # 自定义请求处理声明
├── http.cpp # HTTP 服务器核心实现
├── http.h # HTTP 服务器声明
├── main.cpp # 主程序入口
├── CMakeLists.txt # CMake 构建配置
├── run.sh # 快速构建脚本
└── README.md # 项目说明文档


## 快速开始

### 编译项目


# 方法一：使用构建脚本（推荐）
./run.sh

# 方法二：手动 CMake 构建
mkdir build && cd build
cmake .. -G "Unix Makefiles"
make
# 可执行文件会生成在项目根目录或 bin/ 目录
运行服务器
bash
# 运行服务器（默认端口 8888）
./thttpd.out

# 或指定端口
./thttpd.out 8080
访问测试
在浏览器中访问：

http://localhost:8888/ - 访问首页

http://localhost:8888/index.html - 静态页面

http://localhost:8888/login.html - 登录页面

http://localhost:8888/post.html - POST 请求测试页面

测试 POST 请求
bash
# 使用 curl 测试 POST 请求
curl -X POST http://localhost:8888/api/submit \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "name=test&value=123"
核心功能
支持的 HTTP 方法
方法	用途	示例
GET	获取静态资源（HTML/CSS/JS/图片）	GET /index.html
GET (带参数)	带查询字符串的请求，触发动态处理	GET /search?q=hello
POST	提交表单数据，触发动态处理	POST /api/login
请求处理流程
text
客户端请求
    ↓
TCP 监听接受 (init_server)
    ↓
handler_msg 解析请求行（方法、URL、查询字符串）
    ↓
build_file_path 构建文件路径
    ↓
check_file_exists 检查文件是否存在
    ↓
    ├── 需要动态处理（带参数的GET / POST）
    │       ↓
    │   handle_request（读取 Content-Length 和 Body）
    │       ↓
    │   parse_and_process（业务逻辑处理）
    │
    └── 静态文件（普通GET）
            ↓
        clear_header（清空请求头）
            ↓
        echo_www（使用 sendfile 发送文件）
    ↓
关闭客户端连接（keep-alive? 当前为短连接）


技术特点
语言: C++11

网络: POSIX Socket API

并发: 多线程（每个请求一个线程）

文件传输: sendfile 零拷贝

构建: CMake + Make
