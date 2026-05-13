#include <cctype>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>   // for memset, strcmp etc.
#include <cstdio>    // for printf, perror
#include <cstdlib>   // for atoi
#include <sys/stat.h>// for stat
#include <fcntl.h>   // for open
#include <sys/sendfile.h>
#include<libgen.h>

// 原有头文件
#include "http.h"
#include "custom_handle.h"

//声明初始化服务器函数
int init_server(int _port)
{
    //创建套接字
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
    {
        perror("socke error");
        return  -1;
    }

    //端口号快速重用
    int reuse = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))==-1)
    {
        perror("setsockopt error");
        return -1;
    }

    //绑定ip和端口号
    //填充地址信息结构体
    struct sockaddr_in local; 
    local.sin_family = AF_INET;    //协议族
    local.sin_port = htons(_port);    //端口号
    local.sin_addr.s_addr = INADDR_ANY;     //表示可以接收任意主机的连接请求

    //绑定工作
    if(bind(sock, (struct sockaddr*)&local, sizeof(local))==-1)
    {
        perror("bind error");
        return -1;
    }


    //启动监听
    if(listen(sock, 128) == -1)
    {
        perror("listen error");
        return -1;
    }

    //程序执行至此，说明服务器创建成功
    return sock;

}


//自定义从客户端套接字中读取一行信息函数
int get_line(int sock, char *buf)
{
    char ch = '\0';     //读取一个字符
    int i=0;     //填充变量，用于填充buf，所以从0开始
    int ret = 0;      //记录读取数据的返回值

    while(i<SIZE && ch!='\n')      //当buf没有满并且，没有读取到换行时，继续读取
    {
        ret = recv(sock, &ch, 1, 0);      //从缓冲区中读取一字节数据放入ch中
        if(ret>0 && ch=='\r')
        {
            //下一个字符可能是'\n'
            int s = recv(sock, &ch, 1, MSG_PEEK);     //将下一个字符拿出来看看
            if(s>0 && ch=='\n')
            {
                recv(sock, &ch, 1, 0);     //将回车读取出来
            }else
            {
                ch = '\n';        //直接放入回车，结束
            }
        }

        //程序执行至此，表示该字符需要放入字符串中
        buf[i] = ch;      //将读取下来的字符放入数组中
        i++;             //继续填充下一个位置
    }
    buf[i] = '\0';        //将字符串补充完整
    return i;          //将字符数组中字符串实际长度返回
}

//定义清空头部的函数
static void clear_header(int sock)
{
    char buf[4096] = "";    //读取消息的容器
    int ret = 0;      //读取的个数
    do
    {
        ret = get_line(sock, buf);          //循环将每一行数据全部读取出来
    } while (ret!=1 && strcmp(buf, "\n")!=0);
    
}



//定义展示404界面的函数
static void show_404(int sock)
{
    //将获取的消息头部全部清除掉，上面解析数据时，只解析了请求行
    clear_header(sock);

    //组装首部信息
    const char *msg = "HTTP/1.1 404 Not Found\r\n";
    //发送首部
    send(sock, msg, strlen(msg), 0);
    send(sock, "\r\n", strlen("\r\n"), 0);     //发送一个空行，切记切记

    //打开一个html页面文档
    struct  stat st;     //定义一个文件状态结构体
    stat("../wwwroot/404.html", &st);         //获取wwwroot目录下的404界面信息，主要使用该信息中文件大小的成员
    int fd = open("../wwwroot/404.html", O_RDONLY);    //以只读的形式打开文件
    if(fd == -1)
    {
        perror("open 404 error");
        return;
    }

    //将整个文件发送出去
    //函数原型：ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
    //功能：将一个文件描述符中的数据发送到另一个文件描述符中
    //参数1：要被拷贝到的文件描述符
    //参数2：从哪个文件中拷贝
    //参数3：从该文件的哪个位置开始拷贝
    //参数4：拷贝的文件大小
    sendfile(sock, fd, NULL, st.st_size);
    close(fd);
    
}


//定义处理错误信息的响应
//参数1：套接字文件描述符
//参数2：错误码
void echo_error(int sock, int err_code)
{
    //对错误码进行多分枝选择
    switch(err_code)
    {
        case 403:
            break;
        case 404:
            show_404(sock);     //调用展示404界面
            break;
        case 405:
            break;
        case 500:
            break;
        default:
            break; 
    }
}


//定义响应普通页面的函数
static int echo_www(int sock, const char *path, size_t s)
{
    //以只读的形式打开文件
    int fd = open(path, O_RDONLY);
    if(fd == -1)
    {
        perror("open error");
        return -1;
    }

    //组装响应首部
    const char *msg = "HTTP/1.1 200 OK\r\n";
    send(sock, msg, strlen(msg), 0);        //发送给客户端网页
    send(sock, "\r\n", strlen("\r\n"), 0);  //发送空行

    //将整个文件发送给网页端
    if(sendfile(sock, fd, NULL, s) ==-1)
    {
        echo_error(sock, 500);        //返回错误界面
        return -1;
    }

    close(fd);
    return 0;
}

//定义处理POST请求或携带数据的GET请求
static int handle_request(int sock, const char *method,const char * path, const char * querry_string)
{
    char line[SIZE] = "";     //存储一行信息
    int ret = 0;         //接收读取的内容
    int content_len = -1;    //文本长度

    //判断是什么请求
    if(strcasecmp(method, "GET") == 0)         //GET请求
    {
        //清空消息报头
        clear_header(sock);
    }else
    {
        //获取POST请求的参数大小
        do
        {
            ret = get_line(sock, line);
            if(strncasecmp(line, "content-length", 14) == 0)       //找到该长度
            {
                content_len = atoi(line+16);    //将数字字符串转换为整数
            }
        }while(ret!=1 && (strcmp(line, "\n"))!=0);
    }

    //输出相关数据
    printf("method = %s\n", method);
    printf("query_string = %s\n", querry_string);
    printf("content_len = %d\n", content_len);

    char req_buf[4096] = "";      //用于回复的消息
    //如果是POST请求，那么肯定会携带数据，需要将数据解析出来
    if(strcasecmp(method, "POST") == 0)
    {
        int len = recv(sock, req_buf, content_len, 0);      //将所需信息读取出来
        printf("len = %d\n", len);      
        printf("req_buf = %s\n", req_buf);
    }

    //发送给客户端消息
    const char *msg = "HTTP/1.1 200 OK\r\n\r\n";       //回复报文响应首部
    send(sock, msg, strlen(msg), 0);

    //请求交给自定义的函数来处理业务逻辑
    parse_and_process(sock, querry_string, req_buf);

    return 0;
}

// 定义解析请求行的结构体
typedef struct {
    char method[32];        // 请求方法
    char url[SIZE];         // 请求URL
    char *query_string;     // 查询字符串指针
    int need_handle;        // 是否需要自定义处理
} RequestInfo;

// 解析请求行，提取方法和URL
static int parse_request_line(int sock, RequestInfo *req_info)
{
    char buf[SIZE] = "";
    int count = get_line(sock, buf);
    
    if (count <= 0) {
        return -1;
    }

    // 提取请求方法
    int i = 0;
    int k = 0;
    for (i = 0; i < count; i++) {
        if (isspace(buf[i])) {
            break;
        }
        req_info->method[k++] = buf[i];
    }
    req_info->method[k] = '\0';

    // 跳过空格
    while (isspace(buf[i]) && i < SIZE) {
        i++;
    }

    // 验证请求方法
    if (strcasecmp(req_info->method, "GET") != 0 && 
        strcasecmp(req_info->method, "POST") != 0) {
        printf("method error\n");
        close(sock);
        return -1;
    }

    // 判断是否为POST请求
    if (strcasecmp(req_info->method, "POST") == 0) {
        req_info->need_handle = 1;
    } else {
        req_info->need_handle = 0;
    }

    // 提取URL和查询字符串
    int t = 0;
    req_info->query_string = NULL;
    for (; i < SIZE; i++) {
        if (isspace(buf[i])) {
            break;
        }
        
        if (buf[i] == '?') {
            req_info->query_string = &req_info->url[t + 1];
            req_info->url[t] = '\0';
        } else {
            req_info->url[t] = buf[i];
        }
        t++;
    }
    req_info->url[t] = '\0';

    // 如果是GET请求且带有查询参数，也需要自定义处理
    if (strcasecmp(req_info->method, "GET") == 0 && req_info->query_string != NULL) {
        req_info->need_handle = 1;
    }

    printf("url = %s\n", req_info->url);
    printf("query_string = %s\n", req_info->query_string);

    return 0;
}

// 构建文件路径
static void build_file_path(const char *url, char *path, size_t path_size)
{
    char exe_path[256];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len == -1) {
        perror("readlink error");
        return;
    }
    exe_path[len] = '\0';

    char *dir = dirname(exe_path);
    dir = dirname(dir);

    snprintf(path, path_size, "%s/wwwroot%s", dir, url);

    // 如果请求的是目录，默认返回index.html
    if (path[strlen(path) - 1] == '/') {
        strncat(path, "index.html", path_size - strlen(path) - 1);
    }

    printf("path = %s\n", path);
}

// 检查文件是否存在
static int check_file_exists(const char *path, struct stat *st)
{
    if (stat(path, st) == -1) {
        printf("can not find file\n");
        return -1;
    }
    return 0;
}

// 用于处理客户端信息函数的定义
// sock表示客户端套接字文件描述符
// 接收请求 → 解析请求行 → 提取方法/URL → 区分GET/POST → 路由分发 → 返回响应
int handler_msg(int sock)
{
    // 定义容器存储客户端发来的数据
    char del_buf[SIZE] = "";

    // 读取客户端发来的请求（仅查看，不读走）
    recv(sock, del_buf, SIZE, MSG_PEEK);

#if 1
    // 打印客户端发来的数据
    printf("-------------------------------------------------\n");
    printf("%s\n", del_buf);
    printf("-------------------------------------------------\n");
#endif

    // 解析请求行
    RequestInfo req_info;
    memset(&req_info, 0, sizeof(RequestInfo));
    
    if (parse_request_line(sock, &req_info) != 0) {
        return -1;
    }

    // 构建文件路径
    char path[SIZE] = "";
    build_file_path(req_info.url, path, sizeof(path));

    // 检查文件是否存在
    struct stat st;
    if (check_file_exists(path, &st) != 0) {
        echo_error(sock, 404);
        close(sock);
        return -1;
    }

    // 根据请求类型分发处理
    if (req_info.need_handle == 1) {
        // POST请求或带参数的GET请求，需要自定义处理
        handle_request(sock, req_info.method, path, req_info.query_string);
    } else {
        // 普通GET请求，直接返回文件内容
        clear_header(sock);
        echo_www(sock, path, st.st_size);
    }

    // 关闭客户端连接
    close(sock);
    return 0;
}