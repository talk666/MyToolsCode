#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/time.h>
#include <time.h>

#define PORT (6789)
#define IP   "127.0.0.1"//INADDR_ANY
#define RAND_FILE_SIZE (10 * 1024 * 1024)

#define MESSAGE_LENGTH (4096)
unsigned char sendbuf[MESSAGE_LENGTH], recvbuf[MESSAGE_LENGTH];

int tcp_socketid;

void Stop(int signo)
{
    printf("CTRL+C press\r\n");
    close(tcp_socketid);
    exit(0);
}

int main()
{

    struct timeval time1, time2;
    uint64_t       ts1, ts2, ts;
    
    signal(SIGINT, Stop);
    tcp_socketid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//第三个参数一般设置为0 接口会自动推演出tcp还是udp连接
    if (tcp_socketid == -1) {
        printf("socket creat faild! ret=%d\r\n", errno);
        exit(1);
    }
    printf("socket creat secceed! tcp_socketid=%d\r\n", tcp_socketid);

    //htons将一个unsigned short整型转换为大端字节序
    //htonl将一个unsigned long整型转换为大端字节序
    //sin_port 在内核中定义typedef unsigned short __u16;
    //所以用htons  htonl会数据出错连接失败

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);//htos 和 htonl
    serverAddr.sin_addr.s_addr = inet_addr(IP);

    int ret = connect(tcp_socketid, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret == -1) {
        printf("socket connect faild! ret=%d\r\n", errno);
        exit(1);
    }
    printf("success to connect server...[ret-%d]\n",ret);

#if 1
    while (1)
    {
        memset(sendbuf, 0, MESSAGE_LENGTH);
        memset(recvbuf, 0, MESSAGE_LENGTH);


        printf("<<<<send getrandom len-->enter quit to exit:");

        scanf("%s",sendbuf);

        if(strcmp(sendbuf, "quit") == 0){
            break;
        }
   
        ret = send(tcp_socketid, sendbuf, strlen(sendbuf), 0);
        if(ret <= 0 ){
            printf("the connection is disconnection!\n");
            break;
        }
        
        printf(">>>send message-->%s\r\n", sendbuf);

        int data_len = recv(tcp_socketid, recvbuf, 32, 0);

        recvbuf[data_len] = '\0';
        xprint_hex("echo recv random data-->\r\n", 0, recvbuf, 32);
    }
#elif 0
    memcpy(sendbuf, "123", 3);
    int loop_num = 0;
    
    gettimeofday(&time1, NULL);

    for (loop_num = RAND_FILE_SIZE; loop_num >0; loop_num -= 32)
    {
        ret = send(tcp_socketid, sendbuf, strlen(sendbuf), 0);
        if(ret <= 0 ){
            printf("the connection is disconnection!\n");
            break;
        }
        
     //   printf(">>>send message-->%s\r\n", sendbuf);

        int data_len = recv(tcp_socketid, recvbuf, 32, 0);

        // recvbuf[data_len] = '\0';
      //  xprint_hex("echo recv random data-->\r\n", 0, recvbuf, 32);
    }

    gettimeofday(&time2, NULL);

    ts1 = time1.tv_sec * 1000 + time1.tv_usec / 1000;
    ts2 = time2.tv_sec * 1000 + time2.tv_usec / 1000;

    ts = ts2 - ts1;

    printf("time left = %.2fs\n", ts / 1000.0);
    printf("speed = %dMbps\n", RAND_FILE_SIZE / 1024 / 1024 * 8 * 1000 / ts);
#else

    memcpy(sendbuf, "10485760", 8);
    
    gettimeofday(&time1, NULL);

    ret = send(tcp_socketid, sendbuf, strlen(sendbuf), 0);
    if(ret <= 0 ){
        printf("the connection is disconnection!\n");
    }
    
    recv(tcp_socketid, recvbuf, 32, 0);
    gettimeofday(&time2, NULL);

    ts1 = time1.tv_sec * 1000 + time1.tv_usec / 1000;
    ts2 = time2.tv_sec * 1000 + time2.tv_usec / 1000;

    ts = ts2 - ts1;

    printf("time left = %.2fs\n", ts / 1000.0);
    printf("speed = %dMbps\n", RAND_FILE_SIZE / 1024 / 1024 * 8 * 1000 / ts);
#endif
    close(tcp_socketid);

    return 0;
}