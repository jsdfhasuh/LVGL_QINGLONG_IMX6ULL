#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include "cJSON.h"
#include "http.h"

#define MAX_SIZE 1024
static int i = 0;
static int j =0;
static int sockfd;
static int resource_length = 10240;
static int resource_new_length = 0;
static int retry_time = 3;

static http_data *data;

void error(const char *msg)
{
    perror(msg);
}

int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("Error opening socket");
    }
    printf("socketfd: %d \r\n", sockfd);
    return sockfd;
}

void close_socket(int sockfd)
{
    if (sockfd >= 0)
    {
        close(sockfd);
    }
}

void cancel_signal_handling(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        error("Error getting file descriptor flags");
    }

    // 清除 FASYNC 标志
    flags &= ~FASYNC;

    if (fcntl(fd, F_SETFL, flags) == -1)
    {
        error("Error setting file descriptor flags");
    }

    // 将信号处理程序设置为默认处理程序
    signal(SIGIO, SIG_DFL);
}

void send_http_post_request(int sockfd, const char *host, const char *path,const char *data,const char *custom_headers)
{
    char request[MAX_SIZE];


    snprintf(request, sizeof(request),
            "POST %s HTTP/1.1\n"
            "Host: %s\n"
            "%s"  // 插入自定义请求头
            "Connection: close\n"
            "\n"
            "%s",
            path, host,custom_headers,data);
    printf("%s \n", request);

    // 发送post请求
    if (write(sockfd, request, strlen(request)) < 0)
    {
        error("Error writing to socket");
    }
    printf("send request\n");
}

void send_http_request(int sockfd, const char *host, const char *path,const char *custom_headers,int port,const char *action,const char *body)
{
    char request[MAX_SIZE];
        snprintf(request, sizeof(request),
             "%s %s HTTP/1.1\n"
             "Host: %s:%d\n"
             "%s"  // 插入自定义请求头
             "\n"
             "%s", // 插入请求体
             action,path, host,port, custom_headers,body);
    printf("request is \n%s \n", request);

    // 发送get请求
    if (write(sockfd, request, strlen(request)) < 0)
    {
        error("Error writing to socket");
    }
    printf("send request\r\n");
}



void set_nonblocking(int sockfd,int get)
{
    int flags;

    // 获取当前文件描述符标志
    if ((flags = fcntl(sockfd, F_GETFL, 0)) == -1)
    {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }

    // 设置非阻塞标志
    flags |= O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) == -1)
    {
        perror("fcntl F_SETFL O_NONBLOCK");
        exit(EXIT_FAILURE);
    }
    if (fcntl(sockfd, F_SETOWN, getpid()) == -1) {
        perror("fcntl F_SETOWN");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (get)
    {
        flags |= O_ASYNC;
        // 允许套接字产生SIGIO信号
        if (fcntl(sockfd, F_SETFL, flags) == -1) {
            perror("fcntl F_SETFL O_ASYNC");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }
}

// 扩大http_data里面的response
void copy_raw_data_to_responsedata(char *buffer,ssize_t buffer_length)
{
    if (resource_new_length + buffer_length > resource_length)
    {
        resource_length = resource_length * 2;
        data->response = (char *)realloc(data->response, resource_length);
    }
    strncat(data->response, buffer,buffer_length);
    resource_new_length += buffer_length;
}

void change_response_to_json()
{
    for (int i = 0; i < strlen(data->response);i++)
    {
        if (data ->response[i] == '\r' && data ->response[i + 1] == '\n' && data ->response[i + 2] == '\r' && data ->response[i + 3] == '\n')
        {
            data ->body = data ->response + i + 4;
            break;
        }
    }
    cJSON *json = cJSON_Parse(data->body);
    if (json == NULL)
    {
        printf("response is not json");
    }
    else{
        printf("response is json\n");
        data -> response_json = json;
    }
}

void sigio_handler(int signo) {
    char buffer[1024];
    j += 1;
    bool real_read = false;
    printf("Received signal time is %d\n", j);
    while (1) {
        i += 1;
        ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            buffer[n] = '\0'; // 确保字符串以空字符结尾
            printf("Received data async:\n %s\n", buffer);
            printf("Received data copy to response\n");
            copy_raw_data_to_responsedata(buffer,n);
            real_read = true;
        } else if (n == 0) {
            // 连接关闭
            printf("Connection closed by the peer.\n");
            data->fisnish_status = 1;
            break;
        } else {
            perror("recv");
            if (retry_time != 0)
            {
                printf("retry time is %d\n",retry_time);
                retry_time -= 1;
            }
            else{
                data->fisnish_status = 1;
                retry_time = 3;
                break;
            }

        }
    }
    if (!real_read) {
        printf("No data to read. final_read\n");
        data->fisnish_status = 1;
    }
}

// http client 的主函数
http_data* http_main(char * host,char * path,int port,char * action,int get,const char *custom_headers,const char *body)
{
    // 设置response的大小
    data = (http_data *)malloc(sizeof(http_data));
    data->response = (char *)calloc(resource_length,sizeof(char));
    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&server_addr.sin_zero, 0, 8); // 填充，使结构体大小与 sockaddr 一致
    server_addr.sin_family = AF_INET;
    inet_aton(host, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    // 创建socket
    sockfd = create_socket();
    data->sockfd = sockfd;

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        error("Error connecting");
    }
    // 设置套接字属性
    set_nonblocking(sockfd,get);

    // 发送HTTP get请求
    send_http_request(sockfd, host, path,custom_headers,port,action,body);
    if (get)
    {
        // 注册SIGIO信号处理函数
        struct sigaction sa;
        sa.sa_handler = sigio_handler;
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);
        if (sigaction(SIGIO, &sa, NULL) == -1) {
            perror("sigaction");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        while (data->fisnish_status == 0)
        {
            printf("request no finish");
            sleep(1);
        }
        cancel_signal_handling(sockfd);
        change_response_to_json();
        close(sockfd);
    }
    else
    {
        close(sockfd);
    }
    return data;
}
#if 0
int main()
{
    // 域名或IP地址连接

	// 域名情况下的与服务端建立socket连接
    const char *host = "192.168.1.165";
    const char *path = "/open_ai_flag";
    int port = 8000;
    int flags;

    
    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&server_addr.sin_zero, 0, 8); // 填充，使结构体大小与 sockaddr 一致
    server_addr.sin_family = AF_INET;
    inet_aton(host, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);


    // 创建socket
    sockfd = create_socket();

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        error("Error connecting");
    }
    // 设置套接字属性
    set_nonblocking(sockfd);
        
    // 注册SIGIO信号处理函数
    struct sigaction sa;
    sa.sa_handler = sigio_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGIO, &sa, NULL) == -1) {
        perror("sigaction");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    // 发送HTTP get请求
    send_http_get_request(sockfd, host, path);

    // 接收并打印HTTP响应
    //receive_http_response(sockfd);
        // 主循环
    while (1) {
        sleep(10);
        printf("sleep\n,time is %d\n",i);
    }

    // 关闭socket
    close_socket(sockfd);

    return 0;

}
#endif