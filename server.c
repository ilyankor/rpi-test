// copied from https://stackoverflow.com/q/14915988

#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>

#include <unistd.h>

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

void process_data(const char *data) {
    printf("Message from remote host: %s\n", data);
}

void handle_session(int acc_ds, int max_size) {

    // create text buffer
    char buff[max_size];
    memset(buff, 0, max_size);
    
    // read incoming network data until client disconnect or error
    // check max_size - 1 for null terminator
    while (read(acc_ds, buff, max_size - 1) > 0) {

        // process the data in the buffer
        process_data(buff);
    
        // break if the client sends `quit`
        if (strncmp(buff, "quit", 4) == 0) break;

        memset(buff, 0, max_size);
    }

    // end recieving data
    write(acc_ds, "Reading completed\n", 18);
    close(acc_ds);
}

void handle_connections(int sock_ds, int max_size) {

    // create client IPv4 address
    struct sockaddr_in rem_addr;
    socklen_t length = sizeof(rem_addr);

    // main handling loop
    while (1) {

        // check for client connections
        int acc_ds = accept(sock_ds, (struct sockaddr *)&rem_addr, &length);
        if (acc_ds == -1) {
            perror("Accept error");
            continue;
        }

        // child process spawned
        if (fork() == 0) {
            // child process does not need listening socket
            close(sock_ds);
            
            // handle client session
            handle_session(acc_ds, max_size);

            exit(0);
        }
        // otherwise, parent process so close it
        else {
            close(acc_ds);
        }
    }
}

int main() {

    // PARAMETERS
    const int port_num = 3333;     // port number
    const int backlog = 5;         // maximum queue size
    const int max_buf_size = 1024; // maximum size of the buffer

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
