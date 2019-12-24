#include <stdio.h>
#include <stdlib.h>     //for system .
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>     //fork, exec 등
#include <string.h>
#include <signal.h>
#include <sys/wait.h>  //wait~

#define BUF_SIZE 512
struct sigaction sa1;



void sig_handler(int signo){
    
    unlink("./namedpipe");
    printf("\nTerminate server after releasing namedpipe\n");
    exit(-1);
}

int main()
{   char filename[BUF_SIZE]; //파일 이름
    char mode; //mode check
    int piped; //pipe director
    int Writebytenum; //write한 byte 수
    int readbyte; //읽을 byte
    pid_t readpid;  //read process
    pid_t writepid; //write process
    int getwait;    //wait
    int fd;         //file director
    char InputString[BUF_SIZE]; //write string
    int FileReadNum;     //file문자 개수    

    if(mkfifo("./namedpipe",0666)==-1)          //pipe생성
    {
        printf("fail to call named pipe\n");
        exit(-1);
    }

    if((piped = open("./namedpipe",O_RDWR))<0)  //pipe 최초 open
    {
        printf("fail to call named pipe \n");
        unlink("./namedpipe");
        exit(-1);
    }

    sa1.sa_handler = sig_handler;      //cirl + c 입력시 handler
   
    sigaction(SIGINT,&sa1,NULL);      //pipe unlink 후 종료
    



    printf("If you want exit server & namedpipe , please push Ctrl + C (Software Interrupt)!\n ");
    printf("***server***\n");
    while(1)
    {
        char buf[BUF_SIZE]={0,}; //pipeBuffer 선언 및 초기화
               
            if(read(piped,buf,BUF_SIZE)<0)           //file access 요청체크
            {
                printf("buffer read fail!\n");
                unlink("./namedpipe");
                exit(-1);
            }
            
            sleep(1);                               //읽는 시간 확보
            
            if(!strncmp(buf,"File Access Please!\n",BUF_SIZE))
            {                                             //요청 확인
                printf("file access request receive!\n");
            }   
            else
            {   
                printf("file access receive fail!\n");
                continue;
            }
        if( read(piped,filename,BUF_SIZE)<0) //file name check
        {
            printf("filename read fail!\n");
            unlink("./namedpipe");
            exit(-1);
        }
        //sleep(1);                               //읽는 시간 확보
        printf("filename : %s\n",filename);

        if( read(piped,&mode,1)<0)              //mode check
        {
            printf("mode read fail!\n");
            unlink("./namedpipe");
            exit(-1);
        }  
        sleep(1);                               //읽는 시간 확보
         printf("mode : %c\n",mode); 
        
        switch(mode)                            //mode 경우 나누기
        {
            case 'r' :
            {   
                if(read(piped,&readbyte,4)<0)      //byte 읽기
                { 
                    printf("byte read fail!\n");
                    unlink("./namedpipe");
                    exit(-1);
                }
                sleep(1);             
                printf("readbyte : %d\n",readbyte);



                if((readpid=fork())==0){                    //read child process 생성
                    
                    if((fd = open(filename,O_RDONLY))<0)      //filename에 맞는 file read mode로 open
                    {
                        printf("FILE NOT FOUND\n");
                        if(write(piped,"FILE NOT FOUND\n",BUF_SIZE)==-1)   //file없음을 client에게 알림
                        {
                            printf("delivering fail!\n");
                            exit(-1);
                        }
                        sleep(1);                           //client 읽을 시간 확보
                        
                        exit(1);                            //readpid exit
                    }
                    

                    if((FileReadNum =read(fd,buf,readbyte))<0)                   //file read
                    {
                        printf("file read fail!\n");
                        unlink("./namedpipe");
                        exit(-1);
                    }
                
                    if(FileReadNum < readbyte)                        //파일안의 문자수와 읽고자하는 문자 수 비교
                        buf[FileReadNum-1] = '\0';                   //buf의 FileReadNumIndex에 null값 넣어서 이후 출력 X
                    
                    else
                        buf[readbyte-1] = '\0';                       //위와 동일
                    

                    if(write(piped,buf,BUF_SIZE)==-1)           //File 내 dataString 전달 끝에 null포함 
                    {
                        printf("dataString Delivering fail!\n");
                        unlink("./namedpipe");
                        exit(-1);
                    }
                    sleep(1);                                   //write 후 server가 읽는 것 방지
                    close(fd);                                  //전달 후 file close   
                    exit(1);                                    //성공 시 child process 종료
                }

                else if(readpid==-1){                       //fork 실패
                    printf("fork fail!!!\n");
                    unlink("./namedpipe");
                    exit(-1);
                }
                else{                                       //parent
                    if((getwait=wait(0))==-1){
                        printf("waitFail!\n");
                        unlink("./namedpipe");
                        exit(-1);
                    }
                }
                break;                                      //read mode 종료
            }
            case 'w' :
            {
                if((Writebytenum = read(piped,InputString,BUF_SIZE))<0)           //string 읽기 & string len check
                {
                    printf("string read fail!\n");
                    unlink("./namedpipe");
                    exit(-1);
                }
                InputString[Writebytenum] = '\n';           //input후 개행되도록 string 끝에 개행문자 추가
                InputString[Writebytenum+1] ='\0';          //개행 뒤에 null 추가
                
                printf("Input string : %s\n",InputString);      //input string 확인

                if((writepid=fork())==0)                //write child process 생성
                {
                    if((fd=open(filename,O_WRONLY | O_CREAT,0666))<0)    //파일 create & write.만들어진 file 권한 666
                    {   

                                                
                        printf("File create and Write fail!\n");
                        write(piped,"File create and Write fail\n",BUF_SIZE);   //실패했음을 client에게 전달
                        sleep(1);                               //client 읽을 시간 확보
                      
                        exit(1);                                //write process exit
                    }

                    lseek(fd,0,SEEK_END);              //file의 끝부터 입력하기 위해 fd끝으로 이동                    

                    

                    if(write(fd,InputString,Writebytenum+1)==-1)  //file에 '\n'포함해서 쓰기
                    {
                        printf("File String Input Fail!\n");
                        unlink("./namedpipe");
                        exit(-1);
                    }
                    else                                        //file에 쓰기 성공
                    {   sleep(1);                               //읽을 시간 확보                           

                        sprintf(buf,"%d",Writebytenum);         //WriteByteNum 문자열로 바꿔서 전달하기.
                       
                       if(write(piped,buf,BUF_SIZE)==-1)        //writebyte 전달
                       {
                           printf("writebytenum delivering fail!\n");       //전달 실패
                           unlink("./namedpipe");
                           exit(-1);
                       }
                       sleep(1);                            //전달 후 본인이 읽는 것 방지
                       close(fd);                           //전달 후 file close
                       exit(1);                             //write child process exit
                    }
                    

                }

                else if(writepid<0)                     //fork 실패
                {
                    printf("fork fail!!!\n");
                    unlink("./namedpipe");
                    exit(-1);
                }

                else                                    //parent
                {
                    
                    if((getwait=wait(0))==-1)           //wait 실패
                    {
                        printf("waitFail!\n");
                        unlink("./namedpipe");
                        exit(-1);
                    }
                }
                break;                      //write mode 종료
            }
            default :
            {
                printf("client sends invalid Input!\n");
                break;
            }

        
        }

    }

    return 0;

}