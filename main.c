
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#define MAXQUEUE 20
#define MYPORT 8000
#define GET 1
#define POST 2
#define CLOSE 0
#define KEEP_ALIVE 1
#define CORRECT 1
#define FAIL_UNDERSTAND 0;
#define ROOT "/Users/wuyilun/Desktop/site"
#define IP "172.20.10.8"
#define  BUFSIZE 3000000
#define URL "/welcome.html"
struct req_info{
    int method;
    char url[51];
    int connect_type;
    int content_length;
};
char * user_name_all[5] = {"IamAwesome","WYLiscool"};
char * psd_all[5] = {"12345","54321"};

int loginCheck(char * usrname, char * psd){
    for(int i =0 ;i<5;i++){
        if(strcmp(user_name_all[i],usrname)==0){
            if(strcmp(psd_all[i], psd) == 0){
                return 1;
            }
            else
                return 0;
        }
    }
    return 0;
}



void showSeg(int clen,char * buf){
    int i = 0;
    while(!(buf[i] == '\r' && buf[i+1] == '\n')){
        while(!(buf[i] == '\r' && buf[i+1] == '\n')){
            printf("%c",buf[i++]);
        }
        printf("%c",buf[i++]);
        printf("%c",buf[i++]);
    }
    i = i+2;
    for(int j = 0;j<clen;j++){
        printf("%c",buf[i+j]);
    }
    printf("PrintFinished\n");
}

int get_filetype(const char * filename,char * filetype){
    int cnt = 0;
    size_t name_len = strlen(filename);
    while(filename[cnt]!='.' && cnt<name_len-1){
        cnt++;
    }
    if(cnt >= name_len -1){
        return 0;//err
    }
    cnt++;
    int i =0;
    while(cnt < name_len){
        filetype[i++] = filename[cnt++];
    }
    filetype[i] = '\0';
    return 1;
}


void num2str(int num,char * s){
    int cnt = 0;
    int remain = num;
    while(remain>0){
        s[cnt] = remain%10 + 48;
        remain = remain/10;
        cnt++;
    }
    
    for(int i = 0;i<cnt/2;i++){
        char temp = s[i];
        s[i] = s[cnt-1-i];
        s[cnt-1-i] = temp;
    }
    s[cnt] = '\0';
}




char * str2lower(char * str){
    int loc = 0;
    while(str[loc] != '\0'){
        if('A'<= str[loc] && str[loc] <= 'Z'){
            str[loc] = str[loc] + 32;
        }
        loc++;
    }
    return str;
}
//不存在该项，返回NULL
char * seg_get_item(const char * item_name,char * item_val,int max_val_size,char * buf,int bufsize){
    int i = 0;
    while( i < bufsize-1 && (buf[i] != '\r' || buf[i+1] != '\n')){
        char name[50];
        int cnt = 0;
        while(cnt < 50 && buf[i] != ':'){
            name[cnt++] = buf[i++];
        }
        if( cnt >=50){
            printf("item too long\n");
            return NULL;
        }
        
        name[cnt] = '\0';
        i = i+2;//skip ':' & ' '
        cnt = 0;
        if(strcmp(name,item_name) == 0){
            while( cnt<max_val_size && i < bufsize-1 && (buf[i] != '\r' || buf[i+1] != '\n') ){
                item_val[cnt++] = buf[i++];
            }
            if( i >= bufsize -1 || cnt >= max_val_size){
                printf("illegal seg format\n");
                return NULL;
            }
            item_val[cnt] = '\0';
            str2lower(item_val);
            return item_val;
        }
        
        while( i < bufsize-1 && (buf[i] != '\r' || buf[i+1] != '\n')){
            i++;
        }
        if( i >= bufsize -1){
            printf("illegal seg format(1)\n");
            return NULL;
        }
        i = i+2;//nextline
        
    }
    return NULL;
}
char * seg_addline(char * buf,const char * msg){
    int i = 0;
    while(msg[i]!='\0'){
        buf[i] = msg[i];
        i++;
    }
    return buf+i;
}
char * seg_get_entity(char * buf,int maxsize, char * entityBuf,int contentLen){
    int i =0;
    while(!(buf[i] == '\r' && buf[i+1] == '\n')){
        while(!(buf[i] == '\r' && buf[i+1] == '\n')){
            i++;
        }
        i = i+2;
    }
    i = i+2;
    char * obuf = buf;
    buf = buf+i;
    int j = 0;
    while((i+j)<maxsize && j<contentLen){
        entityBuf[j] = buf[j];
        j++;
    }
    if((i+j)>=maxsize){
        return NULL;
    }
    return obuf;
}

int  processLogin(char * buf,int maxsize,int clen){
    char usrname[25];
    char psd[25];
    char entityBuf[1000];
    seg_get_entity(buf, maxsize, entityBuf,clen);
    int j = 0;
    while(entityBuf[j++] != '='){
        ;
    }
    int cnt = 0;
    while(entityBuf[j] != '&' && cnt<25){
        usrname[cnt++] =entityBuf[j++];
    }
    usrname[cnt] = '\0';
    cnt = 0;
    while(entityBuf[j++] != '='){
        ;
    }
    while(j<clen && cnt<25){
        psd[cnt++] =entityBuf[j++];
    }
    psd[cnt] = '\0';
    printf("name:%s\n",usrname);
    printf("psd:%s\n",psd);
    if(loginCheck(usrname, psd)){
        printf("登入成功\n");
        return 1;
    }
    return 0;
}
char * seg_add_entity(char * buf, char * entity_buf, int entity_len){
    for(int i = 0; i<entity_len;i++){
        buf[i] = entity_buf[i];
    }
    return buf + entity_len;
}
char * seg_add_status(char * buf,const char * protocal,const char * statusCode,const char * status){
    int i = 0;
    int cnt = 0;
    while(protocal[cnt] != '\0'){
        buf[i++] = protocal[cnt++];
    }
    buf[i++] = ' ';
    cnt = 0;
    while(statusCode[cnt] != '\0'){
        buf[i++] = protocal[cnt++];
    }
    buf[i++] = ' ';
    cnt = 0;
    while(status[cnt] != '\0'){
        buf[i++] = protocal[cnt++];
    }
    buf[i++] = '\r';
    buf[i++] = '\n';
    return buf+i;
}

char * seg_add_item(const char * item_name, const char * item_val, char * sendbuf){
    char item_line[100];
    int cnt = 0;
    while(item_name[cnt] != '\0'){
        item_line[cnt] = item_name[cnt];
        cnt++;
    }
    item_line[cnt++] = ':'; item_line[cnt++] = ' ';
    int loc = cnt;
    cnt=0;
    while(item_val[cnt] != '\0'){
        item_line[loc++] = item_val[cnt++];
    }
    item_line[loc++] = '\r'; item_line[loc++]='\n';
    for(int i = 0; i<loc;i++){
        *(sendbuf+i) = item_line[i];
    }
    return sendbuf + loc;
}


char * read_requestline(char * buf,int maxsize, int * mtd,char * url){
    char method[10];
    char ver[10];
    int loc = 0;
    int cnt = 0;
    
    while(buf[loc] != ' ' && cnt<8){
        method[cnt++] = buf[loc++];
    }
    if(cnt>7){
        return NULL;//报文解析出错
    }
    method[cnt] = '\0';
    cnt = 0;
    loc++;//skip ' '
    while(buf[loc] != ' '&&cnt<50){
        url[cnt++] = buf[loc++];
    }
    if(cnt>=50){
        printf("URL TOO LONG\n");
        return NULL;
    }
    url[cnt] = '\0';
    cnt = 0;
    loc++;//skip ' '
    while((buf[loc] != '\r' || buf[loc+1] != '\n') && loc < maxsize -1){
        ver[cnt++] = buf[loc++];
    }
    if(loc >= maxsize -1){
        return NULL;
    }
    ver[cnt] = '\0';
    loc = loc+2;
    if(strcmp(method,"GET") == 0){
        *mtd = GET;
    }
    else if(strcmp(method, "POST") == 0){
        *mtd = POST;
    }
    else{
        ;
    }
    return buf+loc;
}

char * seg_parse(char * recvbuf,int size, struct req_info * rinfo){
    char * bufloc = read_requestline(recvbuf, size, &(rinfo->method), rinfo->url);
    char item_val[30];
    char item_val_1[30];
    int left = size-(bufloc - recvbuf);
    if(bufloc){
        if(seg_get_item("Connection", item_val,30,bufloc,left) ){
            if(strcmp(item_val,"keep-alive")==0){
                rinfo->connect_type = KEEP_ALIVE;
            }
            else{
                rinfo->connect_type = CLOSE;
            }
            if(seg_get_item("Content-Length", item_val_1, 30, bufloc, left)){
            int length = atoi(item_val_1);
            printf("clen:%d",length);
            rinfo->content_length = length;
            }
        }
        else{
            printf("报文错误");
            bufloc =NULL;
        }
    
    }
    return bufloc;
}

long file_size(FILE * f){
    fseek(f, 0L, SEEK_END);
    long size = ftell(f);
    rewind(f);
    return size;
}

int get_response(int socket_fd, struct req_info * rinfo){
   
    char * file_buf = (char *)malloc(sizeof(char) * BUFSIZE);
    char filename[100] = ROOT;
    char filetype[10];
    get_filetype(rinfo->url, filetype);
    strcat(filename, rinfo->url);
    printf("Opening %s\n",filename);
    FILE * f = fopen(filename, "rb");
    
    if(f){
        printf("Open Succ\n");
        int length = fread(file_buf, 1, BUFSIZE, f);
        
        /*
         for(int i = 0;i<length;i++){
         printf("%c",file_buf[i]);
         }
         */
        char * sendbuf = (char *)malloc(sizeof(char) * BUFSIZE);
        memset(sendbuf, 0, BUFSIZE);
        char length_s[10];
        char * bufloc = NULL;
        num2str((int)length, length_s);
        char * statusline = "HTTP/1.1 200 OK\r\n";
        bufloc = seg_addline(sendbuf, statusline);
        bufloc = seg_add_item("Connection", "Keep-Alive", bufloc);
        bufloc = seg_add_item("Content-Length", length_s, bufloc);
        bufloc = seg_add_item("Content-Type", filetype, bufloc);
        bufloc = seg_addline(bufloc,"\r\n");
        showSeg(length, sendbuf);
        bufloc = seg_add_entity(bufloc, file_buf, length);
       
        int sentbytes;
        if((sentbytes = send(socket_fd, sendbuf, bufloc - sendbuf , 0)) == -1){
            printf("send() Error\n");
            return 0;
        }
        printf("Socket%d: send %d bytes\n",socket_fd,sentbytes);
        
        fclose(f);
    }
    else{
        int sentbytes;
        printf(" %s Not Found\n",filename);
        char sendbuf[10000];
        memset(sendbuf, 0, 10000);
        FILE * f = fopen("/Users/wuyilun/Desktop/site/notfound.html", "rb");
        int length = fread(file_buf, 1, BUFSIZE, f);
        fclose(f);
        char * bufloc = NULL;
        char * statusline = "HTTP/1.1 404 Not Found\r\n";
        bufloc = seg_addline(sendbuf, statusline);
        bufloc = seg_add_item("Connection", "Close", bufloc);
        char s[20];
        num2str(length, s);
        bufloc = seg_add_item("Content-Length", s, bufloc);
        bufloc = seg_add_item("Content-Type", "html", bufloc);
        bufloc = seg_addline(bufloc,"\r\n");
        bufloc = seg_add_entity(bufloc, file_buf, length);
       
        showSeg(length, sendbuf);
        if((sentbytes = send(socket_fd, sendbuf, bufloc - sendbuf , 0)) == -1){
            printf("send() Error\n");
            return 0;
        }
        printf("Socket%d: send %d bytes\n",socket_fd,sentbytes);
    }
    return 1;
}
int responsePost(int socket_fd,const char * status,const char * status_s,const char * newURL, char * entityBuf,int bufsize,int entityLength, const char * contentType){
    char sendbuf[10000];
    memset(sendbuf, 0, 10000);
    if(newURL){
        char filename[100] = ROOT;
        strcat(filename, newURL);
        printf("Socket %d,Opening %s\n",socket_fd,filename);
        FILE * f = fopen(filename, "rb");
        if(f){//if f
            printf("Open Success\n");
            size_t length = fread(entityBuf, 1, bufsize, f);
            char length_s[10];
            num2str((int)length, length_s);
            char * bufloc = NULL;
            char filetype[10];
            get_filetype(filename, filetype);
            bufloc = seg_add_status(sendbuf, "HTTP/1.1", status, status_s);
            bufloc = seg_add_item("Connection", "Keep-Alive", bufloc);
            bufloc = seg_add_item("Content-Length", length_s, bufloc);
            bufloc = seg_add_item("Content-Type", filetype, bufloc);
            bufloc = seg_add_item("Location",newURL, bufloc);
            bufloc = seg_addline(bufloc, "\r\n");
            bufloc = seg_add_entity(bufloc, entityBuf, (int)length);
            int sentbytes;
            if((sentbytes = send(socket_fd, sendbuf, bufloc - sendbuf , 0)) == -1){
                    printf("send() Error\n");
                    return 0;
                }
            printf("Socket%d: send %d bytes\n",socket_fd,sentbytes);
            fclose(f);
        }//end if f
    }
    else{
        char length_s[10];
        num2str(entityLength, length_s);
        char * bufloc = NULL;
        bufloc = seg_add_status(sendbuf, "HTTP/1.1", status, status_s);
        bufloc = seg_add_item("Connection", "Keep-Alive", bufloc);
        bufloc = seg_add_item("Content-Length", length_s, bufloc);
        if(contentType)
            bufloc = seg_add_item("Content-Type", contentType, bufloc);
        bufloc = seg_addline(bufloc, "\r\n");
        bufloc = seg_add_entity(bufloc, entityBuf, entityLength);
        int sentbytes;
        if((sentbytes = send(socket_fd, sendbuf, bufloc - sendbuf , 0)) == -1){
            printf("send() Error\n");
            return 0;
        }
        printf("Socket%d: send %d bytes\n",socket_fd,sentbytes);
    }
    return 1;
}


int responder(int socket_fd){
    int bufsize = 10000;
    char recvbuf[bufsize];
    struct req_info rinfo;
    int rval;
    if( (rval = recv(socket_fd, recvbuf, 10000, 0)) <=0){
        printf("recv() failed/empty recv: %d\n",rval);
        close(socket_fd);
        return 0;
    }
    if(seg_parse(recvbuf, bufsize, &rinfo)){
        switch (rinfo.method) {
            case GET:
                if(get_response(socket_fd, &rinfo) == 0){
                    close(socket_fd);
                    return 0;
                }
                break;
            case  POST:
                if(strcmp(rinfo.url,"/login.c" ) == 0 && processLogin(recvbuf, bufsize,rinfo.content_length)){
                    char * entityBuf = (char *)malloc(BUFSIZE * sizeof(char));
                    responsePost(socket_fd, "201", "Created", "/welcome.html", entityBuf, BUFSIZE, 0, NULL);
                }
                break;
            default:
                return 0;
                break;
    }
    }
    else{
        char sendbuf[200];
        seg_add_status(sendbuf, "HTTP/1.1","400", "Bad Request");
        if(send(socket_fd, sendbuf, 200, 0) == -1){
            return 0;
        }
    }
    return 1;
}


void process(void * con_id){
    
    while(responder(con_id)){
        ;
    }
    
    printf("Socket %d,Disconnected\n",con_id);
}


int main(int argc, const char * argv[]) {
    
    //创建监听套接字
    int server_id = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in servert_addr;
    memset(&servert_addr,0,sizeof(servert_addr));
    servert_addr.sin_family = AF_INET;
    servert_addr.sin_port = htons(MYPORT);
    servert_addr.sin_addr.s_addr = htons(INADDR_ANY);
    if(bind(server_id,(struct sockaddr *)&servert_addr, sizeof(servert_addr))){
        perror("Bind ERROR:");
        exit(EXIT_FAILURE);
    }
    
    if(listen(server_id, MAXQUEUE)){
        printf("Listen() ERROR\n");
        exit(EXIT_FAILURE);
    }
    char s[7];
    num2str(MYPORT, s);
    printf("监听套接字运行 Running Welcome Socket Port: %s\n",s);
    
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    while(1){
        
        int con_id = accept(server_id, (struct sockaddr *)&client_addr, &addrlen);
        if(con_id < 0){
            printf("Accept() ERROR");
        }
        else{
            pthread_t thread;
            if(pthread_create(&thread, NULL,process , (void *) con_id)){
                printf("线程创建失败\n");
            }
            else{
                printf("线程创建成功,套接字号:%d\n",con_id);
                pthread_detach(thread);//线程分离
            }
        }
        
    }
    //close(server_id);
    
}
