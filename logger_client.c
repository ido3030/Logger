#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     
#include <arpa/inet.h>  

#define SERVER_IP "127.0.0.1"  // IP address of the server (localhost for testing)
#define SERVER_PORT 8080       // Port number for the server
#define BUFFER_SIZE 64         // Buffer size for messages (64 bytes for 64-bit messages)
#define MAX_MESSAGES 8         // Maximum number of messages to retrieve

int main() {
    int client_fd;                  // File descriptor for the client socket
    struct sockaddr_in server_addr; // Structure for the server address
    char buffer[BUFFER_SIZE];       // Buffer to store the message to send
    char receive_buffer[BUFFER_SIZE * MAX_MESSAGES];  // Buffer to store received messages

    // Create socket
    if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {  // Create a UDP socket
        perror("socket failed");  /
        exit(EXIT_FAILURE);       
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;                       // Set the address family to AF_INET (IPv4)
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);     // Set the server IP address
    server_addr.sin_port = htons(SERVER_PORT);              // Set the server port number (convert to network byte order)

    while (1) {  // Infinite loop to send messages to the server
        printf("Enter message: ");  // Prompt the user to enter a message
        fgets(buffer, BUFFER_SIZE, stdin);  // Read a message from standard input (stdin)

        // Remove newline character from fgets
        buffer[strcspn(buffer, "\n")] = 0;  // Replace the newline character with null terminator

        // Send message to server
        int bytes_sent = sendto(client_fd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (bytes_sent == -1) {  // Check if message sending failed
            perror("sendto failed");  
            close(client_fd);          
            exit(EXIT_FAILURE);        
        }

        printf("Message sent to %s:%d\n", SERVER_IP, SERVER_PORT);  // Print a message indicating that the message was sent

        if (strncmp(buffer, "READ", 4) == 0) {  // Check if the message is a "READ" command
            memset(receive_buffer, 0, sizeof(receive_buffer));  // Clear the receive buffer
            int bytes_received = recvfrom(client_fd, receive_buffer, sizeof(receive_buffer), 0, NULL, NULL);  // Receive the response from the server
            if (bytes_received == -1) {  // Check if message reception failed
                perror("recvfrom failed");  
                close(client_fd);            
                exit(EXIT_FAILURE);          
            }

            printf("Last messages:\n%s\n", receive_buffer);  // Print the received messages
        }
    }

    // Close the socket
    close(client_fd);  

    return 0;  
}
