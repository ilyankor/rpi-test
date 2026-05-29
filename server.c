// copied from https://stackoverflow.com/q/14915988

#include <netinet/in.h>
#include <sys/socket.h>

//  #include <netdb.h>
//  #include <signal.h>
//  #include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

void handle_connections(int sock_ds, int max_size) {

    // create client IPv4 address
    struct sockaddr_in rem_addr;
    socklen_t length = sizeof(rem_addr);

    // create text buffer
    char buff[max_size];
    
    // main handling loop
    while (1) {

        // check for client connections
        int acc_ds = accept(sock_ds, (struct sockaddr *)&rem_addr, &length);
        if (acc_ds == -1) {
            perror("Accept error");
            continue;
        }
        
        if (fork() == 0) {
            close(sock_ds);

            memset(buff, 0, max_size);
            int num_bytes = read(acc_ds, buff, max_size);
            while (num_bytes > 0) {
                printf("Message from remote host: %s\n", buff);
                if (strcmp(buff, "quit") == 0) {
                    break;
                }
                memset(buff, 0, max_size);
            }

            write(acc_ds, "Reading done", 13);
            close(acc_ds);
            exit(0);
        }
        else {
            close(acc_ds);
        }
    }
}

int main() {

    // PARAMETERS
    int port_num = 3333;     // port number
    int backlog = 5;         // maximum queue size
    int max_buf_size = 1024; // maximum size of the text buffer

    // prevent zombie processes
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDWAIT;  
    sigaction(SIGCHLD, &sa, NULL);

    // initialize the server
    int sock_ds = initialize_server(port_num, backlog);
    if (sock_ds == -1) {
        perror("Server initialization failed");
        exit(1);
    }
    printf("Server successfully initialized, listening on port %d\n", port_num);

    // handle connections from clients
    handle_connections(sock_ds, max_buf_size);

    return 0;
}
