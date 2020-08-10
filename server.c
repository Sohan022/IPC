#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h> 

#define MSGSZ 2048

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

int receive(int *arr,int id){
    int msqid, i;
    key_t key;
    message_buf  rbuf;
    char str[MSGSZ]; 
    int mId;
    key = 1235;
    if ((msqid = msgget(key, 0666)) < 0) {
       return -1;
    }
    int buf_length = sizeof(message_buf) - sizeof(long); 
    if(peek_message(msqid,1)){
        if (msgrcv(msqid, &rbuf, buf_length, 1, 0) < 0) {
            return -1;
        }
        arr[id] = id;
        send("couple",5,id);
        // printf("%s\n", rbuf.mtextId.mtext);
        printf("Terminal %d is connected\n",id);
        int c1 = 0;
        for(i = 1; i < id; ++i){
            if(arr[i] != 0){
                send("couple",4,id);
            }   
        }
        for(i = 1; i <= id; ++i){
            if(arr[i]!=0)
                c1++;
        }
        if(c1 > 0){
           printf("Connected terminal IDs:\n");
            for(i = 1; i <= id; ++i){
                if(arr[i] != 0){
                    printf("ID: %d\n",arr[i]);
                }   
            } 
        }
        else{
            printf("No terminal is connected\n");
        }
        printf("\n");
        return 1;
    } else if(peek_message(msqid,2)){
        if (msgrcv(msqid, &rbuf,buf_length, 2, 0) < 0) {
            return -1;
        }
        arr[rbuf.mtextId.Id] = 0;
        printf("Terminal %d is disconnected\n",rbuf.mtextId.Id);
        int c2 = 0;
        for(i = 1; i < id-1; ++i){
                if(arr[i] != 0){
                    send("uncouple",4,id);
            }
        }
        for(i = 1; i < id; ++i){
            if(arr[i]!=0)
                c2++;
        }
        if(c2 > 0){
            printf("Connected terminal IDs:\n");
            for(i = 1; i < id; ++i){
                if(arr[i] != 0){
                    printf("ID: %d\n",arr[i]);
                }
            }
        }
        else{
            printf("No terminal is connected\n");
        }
        printf("\n");
        return 2;
    
    } else if(peek_message(msqid,3)){
        if (msgrcv(msqid, &rbuf, buf_length, 3, 0) < 0) {
            return -1;
        }
        strcpy(str,rbuf.mtextId.mtext);
        mId = rbuf.mtextId.Id;
        for(i = 1; i < id-1; ++i){
            if(arr[i] != 0)
                send(str,4,mId);
        }
        return 3;
    }

}
    

int main()
{
    int arr[20], i;
    for(i= 0; i < 20; ++i) arr[i] = 0;
    int id = 1;
    printf("server has started...\n\n");
    while(1){
        int check = receive(arr,id);
        if(check == 1)
            id++;
    
    }           
    

    exit(0);
}
