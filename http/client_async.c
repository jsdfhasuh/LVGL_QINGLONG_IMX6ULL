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
#include "debug.h"

#define MAX_SIZE 1024
static int i = 0;
static int j =0;
static int sockfd;
static int resource_length = 10240;
static int resource_new_length = 0;
static int retry_time = 3;

static http_data client_data={
    .body = NULL,
    .response = NULL,
    .response_json = NULL,
};
http_data *client_data_ptr = &client_data;

static void error(const char *msg)
{
    perror(msg);
}

static int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("Error opening socket");
    }
    #if DEBUG
    printf("socketfd: %d \r\n", sockfd);
    #endif
    return sockfd;
}

static void close_socket(int sockfd)
{
    if (sockfd >= 0)
    {
        close(sockfd);
    }
}

static void cancel_signal_handling(int fd)
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


static void send_http_request(int sockfd, const char *host, const char *path,const char *custom_headers,int port,const char *action,const char *body)
{
    char request[MAX_SIZE];
        snprintf(request, sizeof(request),
             "%s %s HTTP/1.1\n"
             "Host: %s:%d\n"
             "%s"  // 插入自定义请求头
             "\n"
             "%s", // 插入请求体
             action,path, host,port, custom_headers,body);
    
    #if DEBUG
    printf("request is \n%s \n", request);
    #endif

    // 发送get请求
    if (write(sockfd, request, strlen(request)) < 0)
    {
        error("Error writing to socket");
    }
    #if DEBUG
    printf("send request finish\r\n");
    #endif
}



static void set_nonblocking(int sockfd,int get)
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
static void copy_raw_data_to_responsedata(char *buffer,ssize_t buffer_length)
{
    if (resource_new_length + buffer_length > resource_length)
    {
        resource_length = resource_length * 2;
        client_data_ptr->response = (char *)realloc(client_data_ptr->response, resource_length);
    }
    strncat(client_data_ptr->response, buffer,buffer_length);
    resource_new_length += buffer_length;
}

static void change_response_to_json()
{
    for (int i = 0; i < strlen(client_data_ptr->response);i++)
    {
        if (client_data_ptr ->response[i] == '\r' && client_data_ptr ->response[i + 1] == '\n' && client_data_ptr ->response[i + 2] == '\r' && client_data_ptr ->response[i + 3] == '\n')
        {
            client_data_ptr ->body = (client_data_ptr ->response) + i + 4;
            break;
        }
    }
    cJSON *json = cJSON_Parse(client_data_ptr->body);
    if (json == NULL)
    {
        printf("response is not json");
    }
    else{
        client_data_ptr -> response_json = json;
        printf("response is json\n");
    }
    //cJSON_Delete(json);
}

static void sigio_handler(int signo) {
    char buffer[1024];
    bool real_read = false;
    #if DEBUG
    printf("Received signal");
    #endif
    while (1) {
        i += 1;
        ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            buffer[n] = '\0'; // 确保字符串以空字符结尾
            #if DEBUG
            printf("Received client_data async:\n %s\n", buffer);
            printf("Received client_data copy to response\n");
            #endif
            copy_raw_data_to_responsedata(buffer,n);
            real_read = true;
        } else if (n == 0) {
            // 连接关闭
            printf("Connection closed by the peer.\n");
            client_data_ptr->fisnish_status = 1;
            break;
        } else {
            //perror("recv");
            if (retry_time != 0)
            {
                //printf("retry time is %d\n",retry_time);
                retry_time -= 1;
            }
            else{
                client_data_ptr->fisnish_status = 1;
                retry_time = 3;
                break;
            }

        }
    }
    if (!real_read) {
        printf("No data to read. final_read\n");
        client_data_ptr->fisnish_status = 1;
    }
}

// http client 的主函数
http_data* http_main(char * host,char * path,int port,char * action,int get,const char *custom_headers,const char *body)
{
    // 设置response的大小
    #if DEBUG
    printf("http_main\n");
    #endif

    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&server_addr.sin_zero, 0, 8); // 填充，使结构体大小与 sockaddr 一致
    server_addr.sin_family = AF_INET;
    inet_aton(host, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);


    client_data_ptr -> response = (char *)calloc(resource_length,sizeof(char));

    // 创建socket
    sockfd = create_socket();
    client_data_ptr->sockfd = sockfd;

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
        #if DEBUG
        printf("register sigio\n");
        #endif
        struct sigaction sa;
        sa.sa_handler = sigio_handler;
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);
        if (sigaction(SIGIO, &sa, NULL) == -1) {
            perror("sigaction");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        #if DEBUG
        printf("client_data->fisnish_status is %d\n",client_data_ptr->fisnish_status);
        printf("register sigio finish\n");
        #endif
        client_data_ptr->fisnish_status = 0;
        while (client_data_ptr->fisnish_status == 0)
        {
            #if DEBUG
            printf("request no finish");
            #endif
            usleep(500000);
        }
        cancel_signal_handling(sockfd);
        change_response_to_json();
        close(sockfd);
    }
    else
    {
        close(sockfd);
    }
    return client_data_ptr;
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