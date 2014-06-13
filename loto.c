#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

static char* host = "www.pilio.idv.tw";
static char *name = "/tmp/mac_access.h";
static int number = 37,port = 80;
static char buff[4096],message[2048],data[2048];
static int connfd = -1;
static FILE *file;

int open_tcp_client(char *ip_addr, unsigned short int port)
{
    int sockfd = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr; 
    struct hostent * remoteHost;

    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return -1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); 

    if( (remoteHost = gethostbyname(ip_addr)) == 0 )
    {
        printf("Error resolving host\n");
        return -2;
    }

    serv_addr.sin_addr.s_addr = ( (struct in_addr *)(remoteHost->h_addr) )->s_addr;

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return -3;
    } 
    connfd = sockfd;
    return sockfd;
}

int close_tcp_client(int clifd) {
    if(clifd < 1) {
        printf("Not a volid connection socket:%d\n", clifd);
        return -1;
    }
    close(clifd);
    connfd = -1;
    return 0;
}

char* str_change(char str[])
{
    int i;

    for(i=0;i < strlen(str);i++)
    {
        if(str[i] >= 'A' && str[i] <= 'Z')
            str[i]+=32;
    }
    return str;
}    

void set_header(int count)
{
    sprintf(message,"GET /ltobig/list.asp?indexpage=%d HTTP/1.1\r\n",count);
    strcat(message,"Host:www.pilio.idv.tw\r\n");
    strcat(message,"User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:29.0) Gecko/20100101 Firefox/29.0\r\n");
    strcat(message,"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n");
    strcat(message,"Accept-Language: zh-tw,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n");
    strcat(message,"Connection: keep-alive\r\n");
    strcat(message, "\r\n\r\n");
}

void save_file(void)
{
    FILE *fp;
    int x = 0;
    char ch[128],output[128];
    char no_1[32],no_2[32],no_3[32],no_4[32],no_5[32],no_6[32];

    fp=fopen("/tmp/file","r");

    while (fgets(buff,sizeof(buff),fp) != NULL)
    {
        if(strstr(buff,"<b>"))
        {
            if(strstr(buff,"#FFEBD7") || strstr(buff,"#BD0000"))
                continue;

            sscanf(buff,"%*[^b]b>%s</b>",data);
            if (strstr(data,"&nbsp;")){
                sscanf(data,"%[^&nbsp;,&nbsp;]&nbsp;,&nbsp;%[^&nbsp;&nbsp;]&nbsp;&nbsp;%[^&nbsp;,&nbsp;]&nbsp;,&nbsp;%[^&nbsp;,&nbsp;]&nbsp;,&nbsp;%[^&nbsp;,&nbsp;]&nbsp;,&nbsp;%[^&nbsp;&nbsp;]&nbsp;&nbsp;",no_1,no_2,no_3,no_4,no_5,no_6);
                fprintf(file,"%s,%s,%s,%s,%s,%s\t",no_1,no_2,no_3,no_4,no_5,no_6);
                x++;
            }else{    
                sscanf(data,"%[^b]b>",ch); 
                if(strncmp(data,"<font",4)){
                    memset(output,0,sizeof(output));
                    strncpy(output,ch,strlen(ch)-2);    
                    fprintf(file,"%s\t",output);
                    x++;
                }
            }
        }    
        if(x == 4)
        {  
            x = 0;  
            fprintf(file,"\n");
        }
    }
    fclose(fp);
}

void get_web(int sockfd)
{
    FILE *fp;
    fp=fopen("/tmp/file","w");

    for(;recv(sockfd, buff, sizeof(buff), 0);)
    {
        fprintf(fp,"%s",buff);
        if (strstr(buff,"</html>"))
            break;
        memset(buff,0,sizeof(buff));
    }
    fclose(fp);
    close_tcp_client(sockfd);
}

void get_information(void)
{
    int sockfd,i = 1;

    file=fopen("/tmp/loto.txt","w");
    while(i <= number)
    {    
        if ((sockfd = open_tcp_client(host,port)) < 0 )
        {
            printf("Error opening socket!\n");
            exit(1);
        }

        printf("Total:[%d]%d\r",number,i); //Progress bar
        fflush(stdout);

        memset(message,0,sizeof(message));
        memset(buff,0,sizeof(buff));

        set_header(i);
        //printf("%s\n",message);

        if( send(sockfd, message, strlen(message), 0) == -1)
        {
            printf("Error in send\n");
            exit(1);
        }

        struct timeval timeout = {1,0};
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));

        get_web(sockfd); //get web information
        save_file(); // save to /tmp/loto.txt
        i++;
    }
    fclose(file);

    printf("Total:[%d]%d\tOK!!\n",number,i);
}

void statistics(void)
{
    FILE *fp; 
    char buffer[512],time[8],day[8],number[32],other[4];
    char one[4],two[4],three[4],four[4],five[4],six[4];
    int i;
    float ball[49] = {0},count = 0;
    fp=fopen("/tmp/loto.txt","r");
    while (fgets(buffer,sizeof(buffer),fp) != NULL)
    {
        sscanf(buffer,"%s\t%s\t%s\t%s",time,day,number,other);
        sscanf(number,"%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]",one,two,three,four,five,six);

        printf("%02d (%.2f %),%02d (%.2f %),%02d (%.2f %),%02d (%.2f %),%02d (%.2f %),%02d (%.2f %) [%.2f %] %02d (%.2f %)\n",atoi(one),(ball[atoi(one)]/count)*100,atoi(two),(ball[atoi(two)]/count)*100,atoi(three),(ball[atoi(three)]/count)*100,atoi(four),(ball[atoi(four)]/count)*100,atoi(five),(ball[atoi(five)]/count)*100,atoi(six),(ball[atoi(six)]/count)*100,((ball[atoi(one)]/count)*100+(ball[atoi(two)]/count)*100+(ball[atoi(three)]/count)*100+(ball[atoi(four)]/count)*100+(ball[atoi(five)]/count)*100+(ball[atoi(six  )]/count)*100)/6,atoi(other),(ball[atoi(other)]/count)*100);
        ball[atoi(one)] +=1;
        ball[atoi(two)] +=1;
        ball[atoi(three)] +=1;
        ball[atoi(four)] +=1;
        ball[atoi(five)] +=1;
        ball[atoi(six)] +=1;
        ball[atoi(other)] +=1;
        //printf("%d,%d,%d,%d,%d,%d\n",atoi(one),atoi(two),atoi(three),atoi(four),atoi(five),atoi(six));
        count++;
    }
    fclose(fp);
    printf("Total: \n");
    for(i = 1;i < 50;i++)
    {    
        printf("%02d = %.2f %\t",i,(ball[i]/count)*100); 
        if(i%4 == 0)
            printf("\n");
    }
    printf("\n");
}

void main( void )
{
    char ch;
    while(1)
    {   
        printf("1.Get Web Data.\n");
        printf("2.Statistics.\n");
        printf("E.Exit.\n");
        printf("Input Option : ");
        ch = getchar();
        switch(ch)
        {    
            case '1':
                get_information();
                break;
            case '2':
                statistics(); 
                break;
            case 'E':
                exit(1);
                break;    
            default:
                printf("\n");
                break;
        }
    }
}
