// copied from https://stackoverflow.com/q/14915988

#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>

#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// global variable for socket descriptor, used for the `close_server` function
int global_sock_ds = -1;

int initialize_server(int port_num, int backlog) {

    // create an IPv4 TCP/IP socket descriptor
    int sock_ds = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ds == -1) {
        perror("Socket creation error");
        return -1;
    }

    // configure the IPv4 server address to accept connections on `port_num`
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num);
    addr.sin_addr.s_addr = INADDR_ANY;

    // bind the socket to the address
    if (bind(sock_ds, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Binding error");
        return -1;
    }

    // start listening for client connections
    if (listen(sock_ds, backlog) == -1) {
        perror("Listen error");
        return -1;
    }

    return sock_ds;
}

void process_data(const char *data) {
    printf("Message from remote host: %s\n", data);
}

// void handle_session(int acc_ds, int max_size) {

//     // create text buffer
//     char buff[max_size];
//     memset(buff, 0, max_size);
    
//     // read incoming network data until client disconnect or error
//     // check max_size - 1 for null terminator
//     while (read(acc_ds, buff, max_size - 1) > 0) {

//         // process the data in the buffer
//         process_data(buff);
    
//         // break if the client sends `quit`
//         if (strncmp(buff, "quit", 4) == 0) break;

//         memset(buff, 0, max_size);
//     }

//     // end recieving data
//     write(acc_ds, "Reading completed\n", 18);
//     close(acc_ds);
// }

void handle_session(int acc_ds, int max_size) {

    // create text buffer
    char buff[max_size];
    memset(buff, 0, max_size);
    
    // read HTTP request
    // check max_size - 1 for null terminator
    if (read(acc_ds, buff, max_size - 1) > 0) {

        // process the data in the buffer
        process_data(buff);
        
        // write HTTP response
        // web browsers require a "200 OK" header before they will display text
        const char *http_response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n" // blank line is required to separate headers from body
            "<h1>Hello from your Server!</h1>\n"
            "<p>Successfully connected.</p>\n";

        // send the page data back to the phone
        write(acc_ds, http_response, strlen(http_response));
    }

    close(acc_ds);
}

// void handle_session(int acc_ds, int max_size) {

//     // create text buffer
//     char buff[max_size];
//     memset(buff, 0, max_size);
    
//     // read incoming network data
//     // check max_size - 1 for null terminator
//     if (read(acc_ds, buff, max_size - 1) > 0) {

//         // process the data in the buffer
//         process_data(buff);
        
//         // write HTTP response
//         // web browsers require a "200 OK" header before they will display text
//         const char *http_response = 
//             "HTTP/1.1 200 OK\r\n"
//             "Content-Type: text/html\r\n"
//             "Connection: close\r\n"
//             "\r\n" // blank line is required to separate headers from body
//             "<h1>Hello from your Server!</h1>\n"
//             "<p>Successfully connected.</p>\n";

//         // send the page data back to the phone
//         write(acc_ds, http_response, strlen(http_response));
//     }

//     close(acc_ds);
// }

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

void close_server(int signum) {

    // no-op, prevents compiler warning
    (void) signum;

    // close an existing socket
    // if it doesn't exist (equals -1), then there's no need to close it
    printf("Closing server");
    if (global_sock_ds != -1) close(global_sock_ds);

    exit(0);
}

int main() {

    // PARAMETERS
    const int port_num = 3333;     // port number
    const int backlog = 5;         // maximum queue size
    const int max_buf_size = 1024; // maximum size of the buffer

    // prevent zombie processes
    struct sigaction zombie_signal;
    zombie_signal.sa_handler = SIG_IGN;
    sigemptyset(&zombie_signal.sa_mask);
    zombie_signal.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &zombie_signal, NULL);

    // initialize the server
    int sock_ds = initialize_server(port_num, backlog);
    if (sock_ds == -1) {
        perror("Server initialization failed");
        exit(1);
    }

    printf("Server successfully initialized, listening on port %d\n", port_num);
    global_sock_ds = sock_ds;

    // close the server when Ctrl+C is pressed
    struct sigaction close_signal;
    close_signal.sa_handler = close_server;
    sigemptyset(&close_signal.sa_mask);
    close_signal.sa_flags = 0;
    sigaction(SIGINT, &close_signal, NULL);

    // handle connections from clients
    handle_connections(sock_ds, max_buf_size);

    return 0;
}
