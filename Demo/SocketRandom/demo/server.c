#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h> //errno
#include <stdlib.h> //exit
#include <pthread.h>
#include <arpa/inet.h>//inet_ntoa函数
#include <sys/socket.h>
#include <netinet/in.h> //协议类型
#include <sys/epoll.h>
#include <unistd.h> //close
#include <fcntl.h>
#include <semaphore.h>

#include "app.h"
//LT/ET模式:
//int epoll_create(int size)创建epoll实例 通过一颗红黑树进行管理检测
//int epoll_ctl(int epfd,int op,int fd,struct epoll_event *event)管理红黑树上的文件描述符
//int epoll_wait(int epfd,struct epoll_event *events,int maxevents,int timeout)检测epoll中是否有就绪的文件描述符


//锁保护 在执行epoll_ctrl的时候对红黑树节点进行增删，使用互斥锁
pthread_mutex_t mtx;  //pthread_mutex_lock(&mtx); pthread_mutex_unlock(&mtx);

//在执行epoll_wait的时候对
pthread_spinlock_t spin_mtx; //pthread_spin_lock(&spin_mtx); pthread_spin_unlock(&spin_mtx);

#define PORT 6789
#define IP   "127.0.0.1"//INADDR_ANY
#define SEND_MESSAGE_LENGTH (4096)
#define RECV_MESSAGE_LENGTH (11)


int tcp_socketid;//socket句柄


struct sockaddr_in sockaddr_t;//服务器
struct sockaddr_in remote_sockaddr_t;//接收客户端信息

pthread_t pid;
pthread_mutex_t mutex;

//epoll相关
int epfd;
struct  epoll_event ev, evs[1024];

void Epoll_Create()
{
    epfd = epoll_create(256);
    if (epfd == -1) {
        printf("epoll_create faild!\r\n");
        exit(4);
    }
}

void Epoll_Ctl_ADD(int fd)
{
    printf("EPOOL_ADD---fd->%d\r\n",fd);
    memset(&ev, 0, sizeof(struct epoll_event));
    ev.events = EPOLLIN | EPOLLET;//EPOLLET模式为不管你写或者读缓存区是否写满或者读完都不再epoll_wait出来，而LT模式只要没写满或者读完下一次还会epoll出来继续，所以ET模式效率会高一些
    ev.data.fd = fd;

    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

void Epoll_Ctl_DEL(int fd)
{
    printf("EPOOL_DEL---fd->%d\r\n",fd);

    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}

void Stop(int signo)
{
    printf("CTRL+C press\r\n");
    close(tcp_socketid);
    
    pthread_mutex_destroy(&mtx);//销毁锁
    pthread_spin_destroy(&spin_mtx);//销毁锁
    exit(0);
}

//监听fd线程函数
void *Create_connect(void *param)
{   
    int accept_id;//客户端交互句柄

    int addrlen = sizeof(struct sockaddr_in);
    memset(&remote_sockaddr_t, 0x00, sizeof(struct sockaddr_in));
    //通信描述符
    accept_id = accept(tcp_socketid, (struct sockaddr *)&remote_sockaddr_t, &addrlen);
    if (accept_id < 0) {
        printf("socket accept faild! ret=%d\r\n", errno);
        goto err;
    } else {
        printf("[ip:%s][port:%d][accept_id:%d]\r\n",inet_ntoa(remote_sockaddr_t.sin_addr),ntohs(remote_sockaddr_t.sin_port),accept_id);
    }

    int flag = fcntl(accept_id, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(accept_id, F_SETFL, flag);
          
    printf("socket get a tcp req secceed! recv message...\r\n");

    //添加到epoll中
    pthread_mutex_lock(&mtx);
    Epoll_Ctl_ADD(accept_id);
    pthread_mutex_unlock(&mtx);
err:
    return NULL;
}

#define RAND_SIZE (1024*1024*10)
void *Conmmunication_mask(void *param)
{
    int ret;
    uint8_t        out[32];

    int fd = (int)param;
    //用于通信的描述符
    unsigned char recvbuf[RECV_MESSAGE_LENGTH];
    unsigned char sendbuf[SEND_MESSAGE_LENGTH];//发送缓存区

    memset(sendbuf, 0, SEND_MESSAGE_LENGTH);
    
    int loop_num;
    while (1)
    {
        memset(recvbuf, 0, RECV_MESSAGE_LENGTH);
        ret = recv(fd, (void *)recvbuf, RECV_MESSAGE_LENGTH - 1, 0);
        if (ret == 0) {
            //客户端已关闭
            pthread_mutex_lock(&mtx);
            Epoll_Ctl_DEL(fd);
            pthread_mutex_unlock(&mtx);
            close(fd);
            goto err;
        } else if (ret < 0){
            if (errno == EAGAIN) {//说明此时小于0是因为设置了超时或者非堵塞 此时缓存区是没有数据的 也就是读取完了
                printf("数据接收已全部完成!\n");
                break;
            }
            //断开连接
            close(fd);
            printf("recv falid!\r\n!\n");
            goto err;
        }
        
        recvbuf[RECV_MESSAGE_LENGTH - 1] = '\0';
        printf("receive message the client Get random number length-->%d\n", atoi(recvbuf));

        //Tudo
/*        FILE *fp;
        
        fp = fopen("srand.bin", "w");
        if (fp == NULL) {
            ferror(fp);
            return -1;
        }

        pthread_spin_lock(&spin_mtx);
        for (loop_num = atoi(recvbuf); loop_num >0; loop_num -= 32)
        {
            ret = srand_sm3_generate(32, out);
            if (ret) {
                printf("init error %x\n", ret);
                pthread_spin_unlock(&spin_mtx);
                return -1;
            }
            //memcpy(sendbuf, out, 32);
            fwrite(out, 1, 32, fp);
        }
        fclose(fp);

        pthread_spin_unlock(&spin_mtx);
*/
        pthread_spin_lock(&spin_mtx);
        ret = srand_sm3_generate(32, out);
        if (ret) {
            printf("init error %x\n", ret);
            pthread_spin_unlock(&spin_mtx);
            return -1;
        }
        pthread_spin_unlock(&spin_mtx);
        memcpy(sendbuf, out, 32);
    }
    xprint_hex("outrandom4\r\n", 0, sendbuf, 32);
    ret = send(fd, (void *)sendbuf, 32, 0);
    if(ret <= 0 ){
        printf("the send is faild!\n");
        goto err;
    }
err:
    return NULL;
}

void Lisen_sock()
{
     //int socket(int af, int type, int protocol);AF_INET 表示 IPv4 地址；AF_INET6 表示 IPv6 地址
    int ret;
    tcp_socketid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_socketid == -1) {
        printf("socket creat faild! ret=%d\r\n", errno);
        exit(1);
    }
  
    //int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
    memset(&sockaddr_t, 0x00, sizeof(struct sockaddr_in));
    sockaddr_t.sin_family = AF_INET;
    sockaddr_t.sin_port = htons(PORT);//端口号 大端

    sockaddr_t.sin_addr.s_addr = inet_addr(IP);//INADDR_ANY可以和任意的主机通信
    ret = bind(tcp_socketid, (struct sockaddr *)&sockaddr_t, sizeof(sockaddr_t));
    if (ret == -1) {
        printf("socket bind faild! ret=%d\r\n", errno);
        exit(1);
    }

   
    //int listen(int sockfd, int backlog); //第二个参数为建立好连接处于ESATABLISHED状态的队列长度 当客户端connect来到时，
    ret = listen(tcp_socketid, 100);//Z最大队列数backlog+1
    if (ret == -1) {
        printf("socket listen faild! ret=%d\r\n", errno);
        exit(1);
    }
}

int main()
{
    int i;

    pthread_mutex_init(&mtx, NULL);
    pthread_spin_init(&spin_mtx, PTHREAD_PROCESS_SHARED);

    Lisen_sock();
    Epoll_Create();
    Epoll_Ctl_ADD(tcp_socketid);

    signal(SIGINT, Stop);

    while (1)
    {
        int num = epoll_wait(epfd, evs, sizeof(evs) / sizeof(evs[0]), -1);//-1为堵塞，单位为微秒
        printf("epoll_waitting... num = %d\r\n", num);
        for (i = 0; i < num; i++) {
            int fd = evs[i].data.fd;
            
            if (fd == tcp_socketid) {
                //客户端连接  
                pthread_create(&pid, NULL, Create_connect, NULL);
                pthread_detach(pid);
            } else {
                //客户端通信
                pthread_create(&pid, NULL, Conmmunication_mask, fd);
                pthread_detach(pid);   
            }
        }
    }
    close(tcp_socketid);

    pthread_mutex_destroy(&mtx);//销毁锁
    pthread_spin_destroy(&spin_mtx);//销毁锁

    printf("close server connection...\n");
    return 0;
}