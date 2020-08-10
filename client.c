#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h> 
#include <signal.h>

#define MSGSZ 2048
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define KMAG  "\x1B[35m"
#define KYEL  "\x1B[33m"
#define KWHT  "\x1B[37m"

typedef struct msgtext{
    char    mtext[MSGSZ];
    int     Id; 
}msgtextId;

typedef struct msgbuf {
    long    mtype;
    msgtextId mtextId;
    
} message_buf;

bool peek_message( int qid, long type )
{
    int result, length;

    if((result = msgrcv( qid, NULL, 0, type,  IPC_NOWAIT)) == -1)
    {
        if(errno == E2BIG)
            return true;
    }
    
    return false;
}

void send(char *str,int type,int id){
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    message_buf sbuf;
    int buf_length;
    key = 1235;
    strcpy(sbuf.mtextId.mtext,str);
    if ((msqid = msgget(key, msgflg )) < 0) {
        perror("msgget");
    }
     
    sbuf.mtype = type;
    sbuf.mtextId.Id = id;
    buf_length = sizeof(message_buf) - sizeof(long);
    if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
       printf ("%d, %ld, %s, %d\n", msqid, sbuf.mtype, sbuf.mtextId.mtext, buf_length);
        perror("msgsnd");
  
    }
}

int receive(int type,int mId){
    int msqid;
    key_t key;
    message_buf  rbuf;
    char str[MSGSZ];
    int buf_length = sizeof(message_buf) - sizeof(long); 
    key = 1235;
    if ((msqid = msgget(key, 0666)) < 0) {
        perror("msgget");
    }
    if(type == 4 && peek_message(msqid,4)){
        if (msgrcv(msqid, &rbuf, buf_length, type, 0) < 0) {
            perror("msgrcv");
        }
        if(rbuf.mtextId.Id != mId){
            strcpy(str,rbuf.mtextId.mtext);
            mId = rbuf.mtextId.Id;
            printf("Terminal: %d %s\n",mId,str);
        }
        else{
            send(rbuf.mtextId.mtext,rbuf.mtype,rbuf.mtextId.Id);
        }
        
    }
    else if(type == 5 && peek_message(msqid,5)){
        if (msgrcv(msqid, &rbuf,buf_length, type, 0) < 0) {
            perror("msgrcv");
        }
        strcpy(str,rbuf.mtextId.mtext);
        mId = rbuf.mtextId.Id;
        printf("ID: %d\n",mId);
    }

    return mId;
}

char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for(i =0; i < strlen(line); i++){

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
			token[tokenIndex] = '\0';
			if (tokenIndex != 0){
				tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0; 
			}
		} else {
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	tokens[tokenNo] = NULL ;
	return tokens;
}

int main(int argc,char *argv[]){
              
	             
	int i,pid;
    char *path = (char *) malloc(sizeof(char) * 100);
    FILE *fp;
    int p[2];
    pipe(p);
    int flag = 0,mId = 0;
	while(1) {			
        char  **tokens;   
        char *line = (char *) malloc(sizeof(char) * MAX_INPUT_SIZE);
        char str[2048] = {0};
		bzero(line, sizeof(line));
		path = getcwd(path, 100);

		if(argc == 2) { 
			if(fgets(line, sizeof(line), fp) == NULL) { 
				break;	
			}
			line[strlen(line) - 1] = '\0';
		} else { 
			printf("%ssohan%s:%s~%s$%s ",KYEL,KWHT,KMAG,path,KWHT);
			scanf("%[^\n]",line);
			getchar();
		}
		line[strlen(line)] = '\n';
		tokens = tokenize(line);
        if(tokens[0] == NULL){
            continue;
        }
            
        else if(strcmp(tokens[0],"couple")==0){
            flag = 1;
            send("couple",1,mId);
            printf(" \n");
            mId = receive(5,mId);
            pid = fork();
            if(pid == 0){
                while(1){
                    receive(4,mId);
                }
            }
        }
        else if(strcmp(tokens[0],"uncouple")==0){    
            flag = 0;
            printf("Disconnected\n");
            send("uncouple",2,mId);
            kill(pid, SIGTERM);
        }
        else if(strcmp(tokens[0],"exit")==0){
            if(flag == 1) send("uncouple",2,mId);
			exit(0);
		}
        else if(strcmp(tokens[0],"cd")==0){
			int result = chdir(tokens[1]);
		  	if(flag == 1) send(line,3,mId);
		}
        else{
            fp = popen(line, "r");
            if (fp == NULL)
                fprintf(stderr,"File not found\n");
            char temp[MAX_INPUT_SIZE];
            strcat(str,line);
            while (fgets(temp, MAX_INPUT_SIZE, fp) != NULL){
                strcat(str,temp);
                printf("%s",temp);
            }
            printf("\n");
            if(flag == 1) send(str,3,mId);
       }
       for(i=0;tokens[i]!=NULL;i++){
            free(tokens[i]);
        }
        free(tokens);
    }
    
    
      
    exit(0);
}