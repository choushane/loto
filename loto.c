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


# define TMP_FILE_NAME "lto1.txt"
# define FILE_NAME "lto.txt"
# define LTO539 "lto539"
# define LTO649 "ltobig"
# define LTO "lto"
# define LTO539_NUM 5
# define LTO649_NUM 6
# define LTO_NUM 6
# define LTO539_TOTAL 39
# define LTO649_TOTAL 49
# define LTO_TOTAL 38
# define MAXNO(__A__, __B__) ((__A__ > __B__) ? __A__ : __B__)
static char* host = "www.pilio.idv.tw";
static int number = 0,port = 80,total = 0,NUM = 0;
static char *mode = NULL;
static char buff[4096],message[2048],data[2048];
static int connfd = -1;
static FILE *file;
static struct loto *head=NULL;
void sort_list(void);
void comparison(char*);
static struct loto {
    int no;
    char date[16];
    int ball[6];
    struct loto *next;
};

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
        printf("Not a volid connection socket:%d\n", clifd);return -1;
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
    sprintf(message,"GET /%s/list.asp?indexpage=%d HTTP/1.1\r\n",mode,count);
    strcat(message,"Host:www.pilio.idv.tw\r\n");
    strcat(message,"User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:29.0) Gecko/20100101 Firefox/29.0\r\n");
    strcat(message,"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n");
    strcat(message,"Accept-Language: zh-tw,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n");
    strcat(message,"Connection: keep-alive\r\n");
    strcat(message, "\r\n\r\n");
}

int get_total(void)
{
    FILE *fp;
    int x = 0;
    char ch[128],output[128];
    char no_1[32],no_2[32],no_3[32],no_4[32],no_5[32],no_6[32];

    fp=fopen("file","r");

    while (fgets(buff,sizeof(buff),fp) != NULL)
    {
        if(strstr(buff,"list.asp?indexpage="))
        {
            if(strstr(buff,"target=\"_self\""))
                continue;
            sscanf(buff,"%*[^>]>%[^</a>]</a>",data);
	    x = MAXNO(x,atoi(data));
        }    
    }
    fclose(fp);
    return x;
}

void save_file(void)
{
    FILE *fp;
    int x = 0;
    char ch[128],output[128];
    char no_1[32],no_2[32],no_3[32],no_4[32],no_5[32],no_6[32];

    fp=fopen("file","r");

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
    fp=fopen("file","w");

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
    int sockfd,i = 1,find = 1;
    if(number == 0){
	find = 0;
	number = 1;
	printf("Finding all page.....\n");
    }	
    file=fopen(FILE_NAME,"w");
    while( i <= number)
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

        if( send(sockfd, message, strlen(message), 0) == -1)
        {
            printf("Error in send\n");
            exit(1);
        }

        struct timeval timeout = {1,0};
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));

        get_web(sockfd); //get web information

	if(find)
	{
            save_file(); // save to /tmp/loto.txt
            i++;
	}else{
	    number = get_total();	   
	    if(number > 1)
	    {
		find = 1;
	        printf("Get All Page : %d\n",number);
	    }
	}
    }
    fclose(file);

    printf("Total:[%d]%d\tOK!!\n",number,i);
    sort_list();
}

int creat_link(char buffer[512])
{
    struct loto *now;
    char time[8],day[16],number[64],other[8];
    char one[4],two[4],three[4],four[4],five[4],six[4];

    now = (struct loto*)malloc(sizeof(struct loto));

    sscanf(buffer,"%s\t%s\t%s\t%s",time,day,number,other);
    sscanf(number,"%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]",one,two,three,four,five,six);

    now->no=atoi(time);
    strncpy(now->date,day,strlen(day));
    now->ball[0] = atoi(one);
    now->ball[1] = atoi(two);
    now->ball[2] = atoi(three);
    now->ball[3] = atoi(four);
    now->ball[4] = atoi(five);
    now->ball[5] = atoi(six);
    now->ball[6] = atoi(other);

    if(head != NULL)
        now->next=head;
    else
        now->next=NULL;
    head=now;
   
   return atoi(time); 
}

void sort_list(void)
{
    FILE *fp;
    struct loto *now;
    char buffer[512];
    int number = 1,max = 0,i;

    fp=fopen(FILE_NAME,"r");
    while (fgets(buffer,sizeof(buffer),fp) != NULL)
    {
        if (creat_link(buffer) > max)
            max = creat_link(buffer);
    }
    fclose(fp);

    fp=fopen(TMP_FILE_NAME,"w");
    now=head;
    for(i = 1;i <= max;i++)
    {
        while(now != NULL)
        {
            if(i == now->no)
            {
                fprintf(fp,"%04d\t%s\t%02d,%02d,%02d,%02d,%02d,%02d\t%02d\n",now->no,now->date,now->ball[0],now->ball[1],now->ball[2],now->ball[3],now->ball[4],now->ball[5],now->ball[6]);
                break;
            }
            now=now->next;
        }
    }
    fclose(fp);

}

float statistics_39(float *ball)
{
    FILE *fp; 
    char buffer[512],time[8],day[8],number[32];
    char one[4],two[4],three[4],four[4],five[4];
    float count = 0;
    fp=fopen(TMP_FILE_NAME,"r");
    while (fgets(buffer,sizeof(buffer),fp) != NULL)
    {
        sscanf(buffer,"%s\t%s\t%s\t",time,day,number);
        sscanf(number,"%[^,],%[^,],%[^,],%[^,],%[^,]",one,two,three,four,five);

        printf("%s \n %02d [%.0f (%.2f %)],%02d [%.0f (%.2f %)],%02d [%.0f (%.2f %)],%02d [%.0f (%.2f %)],%02d [%.0f (%.2f %)] {%.2f %} , %.2f % \n\n",
		      time,
	atoi(one),ball[atoi(one)],(ball[atoi(one)]/(count*NUM))*100,
	atoi(two),ball[atoi(two)],(ball[atoi(two)]/(count*NUM))*100,
	atoi(three),ball[atoi(three)],(ball[atoi(three)]/(count*NUM))*100,
	atoi(four),ball[atoi(four)],(ball[atoi(four)]/(count*NUM))*100,
	atoi(five),ball[atoi(five)],(ball[atoi(five)]/(count*NUM))*100,
	((ball[atoi(one)]/(count*NUM))*100+(ball[atoi(two)]/(count*NUM))*100+(ball[atoi(three)]/(count*NUM))*100+(ball[atoi(four)]/(count*NUM))*100+(ball[atoi(five)]/(count*NUM))*100),((ball[atoi(one)]/(count*NUM))*100+(ball[atoi(two)]/(count*NUM))*100+(ball[atoi(three)]/(count*NUM))*100+(ball[atoi(four)]/(count*NUM))*100+(ball[atoi(five)]/(count*NUM))*100)/NUM);
        ball[atoi(one)] +=1;
        ball[atoi(two)] +=1;
        ball[atoi(three)] +=1;
        ball[atoi(four)] +=1;
        ball[atoi(five)] +=1;
        count++;
    }
    fclose(fp);
    return count;
}

float statistics_49(float *ball)
{
    FILE *fp; 
    char buffer[512],time[8],day[8],number[32],other[4];
    char one[4],two[4],three[4],four[4],five[4],six[4];
    int i;
    float count = 0;
    fp=fopen(TMP_FILE_NAME,"r");
    while (fgets(buffer,sizeof(buffer),fp) != NULL)
    {
        sscanf(buffer,"%s\t%s\t%s\t%s",time,day,number,other);
        sscanf(number,"%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]",one,two,three,four,five,six);

        printf("%s \n %02d [%.0f (%.2f %)],%02d [%.0f (%.2f %)],%02d [%.0f (%.2f %)],%02d [%.0f (%.2f %)],%02d [%.0f (%.2f %)],%02d [%.0f (%.2f %)] {%.2f %} %02d [%.0f (%.2f %)] %.2f %\n\n",
		      time,
	atoi(one),ball[atoi(one)],(ball[atoi(one)]/(count*NUM))*100,
	atoi(two),ball[atoi(two)],(ball[atoi(two)]/(count*NUM))*100,
	atoi(three),ball[atoi(three)],(ball[atoi(three)]/(count*NUM))*100,
	atoi(four),ball[atoi(four)],(ball[atoi(four)]/(count*NUM))*100,
	atoi(five),ball[atoi(five)],(ball[atoi(five)]/(count*NUM))*100,
	atoi(six),ball[atoi(six)],(ball[atoi(six)]/(count*NUM))*100,
	((ball[atoi(one)]/(count*NUM))*100+(ball[atoi(two)]/(count*NUM))*100+(ball[atoi(three)]/(count*NUM))*100+(ball[atoi(four)]/(count*NUM))*100+(ball[atoi(five)]/(count*NUM))*100+(ball[atoi(six)]/(count*NUM))*100)/6,atoi(other),ball[atoi(other)],(ball[atoi(other)]/(count*NUM))*100,((ball[atoi(one)]/(count*NUM))*100+(ball[atoi(two)]/(count*NUM))*100+(ball[atoi(three)]/(count*NUM))*100+(ball[atoi(four)]/(count*NUM))*100+(ball[atoi(five)]/(count*NUM))*100+(ball[atoi(six)]/(count*NUM))*100));
        ball[atoi(one)] +=1;
        ball[atoi(two)] +=1;
        ball[atoi(three)] +=1;
        ball[atoi(four)] +=1;
        ball[atoi(five)] +=1;
        ball[atoi(six)] +=1;
        ball[atoi(other)] +=1;
        count++;
    }
    fclose(fp);
    return count;
}

void statistics(void)
{
    FILE *fp; 
    int i;
    float ball[49] = {0},count = 0,frequency = 0,percent = 0;

    if (!strcmp(mode, LTO649))
	count = statistics_49(&ball);
    else if (!strcmp(mode, LTO539))
	count = statistics_39(&ball);
    //else if (!strcmp(mode,"38-6-10"))
	//count = statistics_38(&ball);

    printf("\n");
    for(i = 1;i <= total;i++)
    {    
        printf("%02d = %.0f (%.2f) %\t",i,ball[i],(ball[i]/count)*100); 
        frequency+=ball[i];
        percent+=(ball[i]/count)*100;
        if(i%4 == 0)
            printf("\n");
    }

    printf("\n");
    printf("Total(%d):  No. %.2f Frequency %.0f Percent %.2f % \n",total,count,frequency/total,percent/total);
    printf("\n");
}

void search_menu(void)
{
    char buff[1024];
    char *delim=",",*str;
    int check,count;
    while(1)
    {    
        check = 1;
        count=0;
        printf("\nPlease enter your number : \n");
        printf("(Example : 1,2,3,4,5,6) \n");
        printf("Your Number : ");
        scanf("%s",buff);
        str = strtok(buff,delim);
        while(str != NULL)
        {
            if (atoi(str) < 0 || atoi(str) > total){
                check = 0;
                printf("(%d) Enter Wrong!!\n",atoi(str));
                break;
            }
            count++;
            str = strtok(NULL,delim);
        }

        if (check == 1 && count == 6)
        { 
            comparison(buff);
        }else
            printf("Number amount wrong!!\n");
    }
}

void link_list(void)
{
    FILE *fp;
    struct loto *now;
    char buffer[512];
    int max = 0;

    fp=fopen(FILE_NAME,"r");
    while (fgets(buffer,sizeof(buffer),fp) != NULL)
    {
        if (creat_link(buffer) > max)
            max = creat_link(buffer);
    }
    fclose(fp);
}

void comparison(char *str)
{
    if(head == NULL)
        link_list();


}

void choose_mode(char **show)
{
    char ch;
    printf("Choose Mode\n");
    printf("A) : 49-6\n");
    printf("B) : 39-5\n");
    printf("C) : 38-6-10\n");
    do{
	ch = getchar();
	switch(ch)
	{    
	    case 'A':
		mode = LTO649;
		*show = "49-6";
		total = LTO649_TOTAL;
		NUM = LTO649_NUM;
		break;
	    case 'B':
		mode = LTO539;
		*show = "39-5";
		total = LTO539_TOTAL;
		NUM = LTO539_NUM;
		break;
	    case 'C':
		mode = LTO;
		*show = "38-6-10";
		total = LTO_TOTAL;
		NUM = LTO_NUM;
		break;
	}
    }while(ch == 'A' || ch == 'B' || ch == 'C');
}

void main( void )
{
    char ch;
    char *show_mode = NULL;
    int file = 0;
    FILE *fp = NULL;

    while(1)
    {   
	if(!mode)
	    choose_mode(&show_mode);
	printf("Play Mode : %s \n",show_mode);
        printf("1.Get Web Data.\n");
        if(fp=fopen(FILE_NAME,"r"))
        {    
            file=1;
            fclose(fp);
        }
        printf("2.Statistics.(%d)\n",file);
        printf("3.Search Number.\n");
        printf("4.Choose Mode.\n");
        printf("D.DEBUG.\n");
        printf("E.Exit.\n");
        printf("Input Option : ");
        ch = getchar();
        switch(ch)
        {    
            case '1':
                get_information();
                break;
            case '2':
                if(file == 1){
                    statistics(); 
		    exit(0);
                }else
                    printf("\n Not File.");
                break;
            case '3':
                search_menu();
                break;
            case '4':
                choose_mode(&show_mode);
                break;
            case 'D':
                sort_list(); 
                break;
            case 'E':
                exit(1);
                break;    
            default:
                printf("\n");
                break;
        }
	system("clear");
    }
}
