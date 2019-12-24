#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 512

int main(int argc,char* argv[])
{
    char filename[BUF_SIZE]; //filename
    char mode;               //권한
    int piped;               //pipe director
    int len;                // buff안의 string 길이
    int readbyte;           //읽을 byte
    char InputString[BUF_SIZE]; //wirteString
    
    if((piped = open("./namedpipe",O_RDWR))<0)    //named pipe 연결
    {
        printf("fail to call named pipe\n");
        exit(-1);
    }
    printf("If you want exit client , please push Ctrl + C (Software Interrupt)!\n ");
    printf("***client***\n");
    while(1)
    {
        char buff[BUF_SIZE]={0,};     //pipebuff 선언 및 초기화


        printf("File_Name input : ");           //file 이름 입력
        fgets(filename,BUF_SIZE,stdin);
        

        printf("mode input('r' : read | 'w' write ) : ");   //r,w mode 입력
        scanf ("%c",&mode);
        while(getchar()!='\n');

        switch(mode)                                   //r,w case 나눔
        {           
            case 'r':
            {
                printf("Input read byte : "); scanf("%d",&readbyte);    //읽을 byte 수 입력
                while(getchar()!='\n');
                
                if(access("./namedpipe",F_OK)==-1)     //namedpipe의 존재여부 체크
                {
                  printf("Namedpipe with server is missing!\n");
                  exit(-1);   
                }

                if(write(piped,"File Access Please!\n",BUF_SIZE)==-1)   //file access 요청
                {
                    printf("file Access request fail!\n");
                    break;
                }
                sleep(1);
                len = strlen(filename);
                filename[len-1] = '\0';             //개행 제거

                if(write(piped,filename,len)==-1)           //file 이름 server에 전달
                {
                   printf("fileName write fail!\n");
                   exit(-1);  
                }

                sleep(1);                                           //write후 client가 읽는것 방지
                
                if(write(piped,&mode,1)==-1)                //mode server에 전달
                {
                    printf("modewrite fail!\n");
                    exit(-1);  
                }
                sleep(1);

                if(write(piped,&readbyte,4)==-1)                      //읽을 byte 수 전달
                {
                    printf("byte write fail!\n");
                    exit(-1);  
                }
            
                sleep(2);                              //write 후 통신 대기

                if(read(piped,buff,BUF_SIZE)<0)                         //결과 읽음
                {
                    printf("result receive fail!\n");
                    exit(-1);
                }
                if(!strncmp(buff,"FILE NOT FOUND\n",BUF_SIZE))         //찾는 file없으면 처음으로
                {
                    printf("FILE NOT FOUND\n");
                    break;
                }
                else                                                //file 찾기 성공 후 string print
                {
                    
                    printf("%s\n",buff);
                    break;
                }
            }
            case 'w':
            {   
                printf("Input String : ");               //입력하고자 하는 string 전달
                fgets(InputString,BUF_SIZE,stdin);
                
                
                if(access("./namedpipe",F_OK)==-1)     //namedpipe의 존재여부 체크
                {
                    printf("Namedpipe with server is missing!\n");
                    exit(-1);   
                }
                if(write(piped,"File Access Please!\n",BUF_SIZE)==-1)   //file access 요청
                {
                    printf("file Access request fail!\n");
                }
                sleep(1);                               //file 요청 받을 시간 확보

                len = strlen(filename);
                 strncpy(buff,filename,len);    
                buff[len-1] = '\0';

                if(write(piped,buff,len)==-1)           //file 이름 server에 전달
                {
                   printf("fileName write fail!\n");
                   exit(-1);  
                }

                sleep(1);                                    //write후 client가 읽는것 방지
                
                if(write(piped,&mode,1)==-1)                //mode를 server에 전달
                {
                    printf("modewrite fail!\n");
                    exit(-1);  
                }
                sleep(1);                              //write 후 통신 대기
                
                len = strlen(InputString);     
                strncpy(buff,InputString,len);
                if(buff[len-1] =='\n')  {        //개행포함
                        buff[len]=='\0';
                        len--;
                } 
                                      
                sleep(1);                               //server가 읽는 시간 확보

                if(write(piped,buff,len)==-1)           //string을 server에 전달 
                {
                    printf("string write fail!\n");
                    exit(-1);
                }

                sleep(2);                                      //write후 client가 읽는것 방지

                if(read(piped,buff,BUF_SIZE)<0)               //result read
                {
                    printf("result receive fail!\n");
                    exit(-1);
                }
                
                if(!strncmp(buff,"File create and Write fail\n",BUF_SIZE))
                {
                    printf("File create and Write fail\n");
                    break;
                }
                else                                        //wirteByte 전달 받기 성공!
                {
                    printf("WriteByte : %s\n",buff);

                    break;                                  //다시 위로
                }
              
              
            }
            default :                                       //mode가 잘못 입력 됬을 시
            {
                printf("Invalid Input!\n");
                break;
            }
           
        }

        
    }

    return 0;

}