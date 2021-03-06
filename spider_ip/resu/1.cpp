#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IPSTR "111.161.22.19"
#define PORT 80
#define BUFSIZE 1024

int main(int argc, char **argv)
{
    int sockfd, ret, i, h;
    struct sockaddr_in servaddr;
    char str1[4096], str2[4096], buf[BUFSIZE], *str;
    socklen_t len;
    fd_set   t_set1;
    struct timeval  tv;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        printf("create net failed, thread will termited---socket error!\n");
        exit(0);
    };

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IPSTR, &servaddr.sin_addr) <= 0 ){
        printf("create net failed, thread will termited--inet_pton error!\n");
        exit(0);
    };

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        printf("link server failed,connect error!\n");
        exit(0);
    }
    printf("success connected!\n");

    //发送数据
    memset(str2, 0, 4096);
    //strcat(str2, "qqCode=474497857");
    str=(char *)malloc(128);
    len = strlen(str2);
    sprintf(str, "%d", len);

    memset(str1, 0, 4096);
    //strcat(str1, "GET /ips138.asp?ip=61.135.169.121&action=2 HTTP/1.1\n");
    strcat(str1, "GET /ips138.asp?ip=111.161.22.19&action=2 HTTP/1.1\n");
    strcat(str1, "Host: www.ip138.com\n");
    strcat(str1, "Content-Type: application/x-www-form-urlencoded\n");
    strcat(str1, "Content-Length: ");
    strcat(str1, str); //128字节
    strcat(str1, "\n\n"); //空行

    strcat(str1, str2);
    strcat(str1, "\r\n\r\n");
    printf("%s\n",str1);

    ret = write(sockfd,str1,strlen(str1));
    if (ret < 0) {
        printf("send failed, fail_code=%d,fail_message='%s'\n",errno, strerror(errno));
        exit(0);
    }else{
        printf("success send,send %d bytes\n\n", ret);
    }

    FD_ZERO(&t_set1);
    FD_SET(sockfd, &t_set1);

    while(1){
        sleep(6);
        tv.tv_sec= 0;
        tv.tv_usec= 0;
        h= 0;
        printf("--------------->1");
        h= select(sockfd +1, &t_set1, NULL, NULL, &tv);
        printf("--------------->2");

        //if (h == 0) continue;
        if (h < 0) {
            close(sockfd);
            printf("select error！\n");
            return -1;
        };

        if (h > 0){
            memset(buf, 0, 4096);
            i= read(sockfd, buf, 4095);
            if (i==0){
                close(sockfd);
                printf("peer closed\n");
                return -1;
            }

            printf("%s\n", buf);
        }
        sleep(6);
    }
    close(sockfd);

    return 0;
}
