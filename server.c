// copied from https://stackoverflow.com/q/14915988

#include <netinet/in.h>
#include <sys/socket.h>
//  #include <netdb.h>

//  #include <sys/signal.h>

//  #include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//  #define MAXSIZE 1024 //max-bytes for read-buffer


int initialize_server(int port_num, int backlog) {

    // create an IPv4 TCP/IP socket descriptor
    int sock_ds = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ds == -1) {
        perror("Socket creation error");
        return -1;
    }
    
    // configure the IPv4 server address to accept connections on port_num
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num);
    addr.sin_addr.s_addr = INADDR_ANY;

    // return variable
    int ret;

    // bind the socket to the address
    ret = bind(sock_ds, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
        perror("Binding error");
        return -1;
    }

    // start listening for client connections
    ret = listen(sock_ds, backlog);
    if (ret == -1) {
        perror("Listen error");
        return -1;
    }

    return sock_ds;
}

int main() {

    int port_num = 3333;
    int backlog = 5;

    int sock_ds = initialize_server(port_num, backlog);
    if (sock_ds == -1) {
        perror("Server initialization failed");
        exit(1);
    }

    printf("Server initialized, listening on port %d!\n", port_num);

    getchar();

    // struct sigaction sa;
    // sa.sa_handler = SIG_IGN;
    // sigemptyset(&sa.sa_mask);
    // sa.sa_flags = SA_NOCLDWAIT;  
    // sigaction(SIGCHLD, &sa, NULL);



    return 0;

    // int sock_ds, ret, length; // server socket descriptor
    // int acc_ds; // client socket descriptor

    // struct sockaddr_in addr; // server address
    // struct sockaddr rem_addr; // generic client address

    // char buff[MAXSIZE];

    // sock_ds = socket(AF_INET, SOCK_STREAM, 0); // => TCP

    // bzero((char *)&addr, sizeof(addr)); //reset struct
    // addr.sin_family = AF_INET;
    // addr.sin_port = htons(25000);
    // addr.sin_addr.s_addr = INADDR_ANY;

    // ret = bind(sock_ds, &addr, sizeof(addr));
    // if(ret == -1){
    //     perror("Binding error");
    //     exit(1);
    // }

    // ret = listen(sock_ds, BACKLOG); // backlog queue
    //     if(ret == -1){
    //     perror("Listen error");
    //     exit(1);
    // }

    // length = sizeof(rem_addr);
    // signal(SIGCHLD, SIG_IGN); //zombie children management

    // /*Busy-waiting (server) and concurrency */
    // while(1){

    //     /*Repeat until success*/
    //     while(acc_ds = accept(sock_ds, &rem_addr, &length) == -1){

    //         if(fork() == 0){ //child-process

    //             close(sock_ds); //unused from child
    //             do{
    //                 read(acc_ds, buff, MAXSIZE);
    //                 printf("Message from remote host:&s\n", buff);

    //             }while(strcmp(buff, "quit") == 0);
    //             /*Transimission completed: server response  */
    //             write(acc_ds, "Reading Done", 10);
    //             close(acc_ds); //socket closed
    //             exit(0); //exiting from child
    //         }
    //         else{
    //             close(acc_ds); //unused from parent
    //         }
    //     }
}
