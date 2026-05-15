# TinyHttpServer - 轻量级 C++ HTTP 服务器

## 项目简介

这是一个基于 C++ 编写的轻量级 HTTP 服务器，支持处理 GET 和 POST 请求，可处理静态文件服务和简单的动态业务逻辑。

## 目录结构
```
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
```

## 快速开始

### 编译项目
#### 方法一：使用构建脚本（推荐）
```
./run.sh
```
#### 方法二：手动 CMake 构建
```
mkdir build && cd build
cmake .. -G "Unix Makefiles"
make
```
#### 可执行文件会生成在项目根目录或 bin/ 目录
运行服务器
bash
#### 运行服务器（默认端口 8888）
```
./thttpd.out
```
#### 或指定端口
```
./thttpd.out 8080
```
访问测试
在浏览器中访问：

http://localhost:8888/ - 访问首页

http://localhost:8888/index.html - 静态页面

http://localhost:8888/login.html - 登录页面

http://localhost:8888/post.html - POST 请求测试页面
