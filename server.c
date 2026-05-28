// copied from https://stackoverflow.com/q/14915988

 #include <netinet/in.h>
 #include <sys/socket.h>
 #include <netdb.h>

 #include <sys/signal.h>

 #include <unistd.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>

 #define BACKLOG 5
 #define MAXSIZE 1024 //max-bytes for read-buffer

void main(){

    int sock_ds, ret, length;
    int acc_ds; //Accept socket descriptor

    struct sockaddr_in addr; //this addres
    struct sockaddr rem_addr; //remote address (generic)

    char buff[MAXSIZE];

    sock_ds = socket(AF_INET, SOCK_STREAM, 0); // => TCP

    bzero((char *)&addr, sizeof(addr)); //reset struct
    addr.sin_family = AF_INET;
    addr.sin_port = htons(25000);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(sock_ds, &addr, sizeof(addr));
    if(ret == -1){
        perror("Binding error");
        exit(1);
    }

    ret = listen(sock_ds, BACKLOG); // backlog queue
        if(ret == -1){
        perror("Listen error");
        exit(1);
    }

    length = sizeof(rem_addr);
    signal(SIGCHLD, SIG_IGN); //zombie children management

    /*Busy-waiting (server) and concurrency */
    while(1){

        /*Repeat until success*/
        while(acc_ds = accept(sock_ds, &rem_addr, &length) == -1){

            if(fork() == 0){ //child-process

                close(sock_ds); //unused from child
                do{
                    read(acc_ds, buff, MAXSIZE);
                    printf("Message from remote host:&s\n", buff);

                }while(strcmp(buff, "quit") == 0);
                /*Transimission completed: server response  */
                write(acc_ds, "Reading Done", 10);
                close(acc_ds); //socket closed
                exit(0); //exiting from child
            }
            else{
                close(acc_ds); //unused from parent
            }
        }
}
