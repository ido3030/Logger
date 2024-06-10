#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     
#include <arpa/inet.h>  

#define SERVER_PORT 8080           // Port number for the server
#define BUFFER_SIZE 64             // Buffer size for messages (64 bytes for 64-bit messages)
#define MAX_MESSAGES 8             // Maximum number of messages to store in the log
#define LOG_FILE "log.txt"         // Log file name

// Function to log a message to a file
void log_message(const char *message) {
    FILE *file = fopen(LOG_FILE, "a");  // Open the log file in append mode
    if (file == NULL) {                 
        perror("fopen failed");         
        return;                         
    }
    fprintf(file, "%s\n", message);     // Write the message to the log file
    fclose(file);                       // Close the log file
}

// Function to retrieve the last n messages from the log file
void retrieve_messages(char *output, int n) {
    FILE *file = fopen(LOG_FILE, "r");  // Open the log file in read mode
    if (file == NULL) {                
        perror("fopen failed");         
        return;                         
    }

    // Read all messages into a buffer
    char messages[MAX_MESSAGES][BUFFER_SIZE];  // Buffer to store the messages
    int count = 0;                             // Counter for the number of messages read
    while (fgets(messages[count % MAX_MESSAGES], BUFFER_SIZE, file) != NULL) {
        count++;                               // Increment the counter for each message read
    }
    fclose(file);                              // Close the log file

    // Extract the last n messages
    int messages_to_retrieve = n < MAX_MESSAGES ? n : MAX_MESSAGES;  // Determine the number of messages to retrieve
    for (int i = 0; i < messages_to_retrieve; i++) {
        int index = (count - messages_to_retrieve + i + MAX_MESSAGES) % MAX_MESSAGES;  // Calculate the index of the message to retrieve
        strcat(output, messages[index]);  // Append the message to the output
    }
}

int main() {
    int server_fd;                      // File descriptor for the server socket
    struct sockaddr_in server_addr;     // Structure for the server address
    struct sockaddr_in client_addr;     // Structure for the client address
    socklen_t client_addr_len = sizeof(client_addr);  // Size of the client address structure
    char buffer[BUFFER_SIZE];           // Buffer to store received messages
    char client_ip_str[INET_ADDRSTRLEN];  // Buffer to store the client's IP address as a string

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {  // Create a UDP socket
        perror("socket failed");       
        exit(EXIT_FAILURE);            
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;            // Set the address family to AF_INET (IPv4)
    server_addr.sin_addr.s_addr = INADDR_ANY;    // Bind to all available network interfaces
    server_addr.sin_port = htons(SERVER_PORT);   // Set the port number (convert to network byte order)

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {  // Bind the socket to the server address
        perror("bind failed");                   // Print error message if bind failed
        close(server_fd);                        // Close the server socket
        exit(EXIT_FAILURE);                      // Exit the program if bind failed
    }

    printf("Server listening on port %d\n", SERVER_PORT);  // Print a message indicating the server is listening

    while (1) {  // Infinite loop to receive messages from clients
        // Receive message from client
        memset(buffer, 0, BUFFER_SIZE);  // Clear the buffer
        int bytes_received = recvfrom(server_fd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&client_addr, &client_addr_len);  // Receive a message
        if (bytes_received == -1) {  // Check if message reception failed
            perror("recvfrom failed");  
            close(server_fd);           
            exit(EXIT_FAILURE);         
        }

        // Convert client IP to string
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_str, INET_ADDRSTRLEN);  // Convert the client's IP address to a string

        printf("Message from %s: %s\n", client_ip_str, buffer);  // Print the received message and the client's IP address

        if (strncmp(buffer, "READ", 4) == 0) {  // Check if the received message is a "READ" command
            int n = atoi(buffer + 5);  // Extract the number of messages to read
            char message_output[BUFFER_SIZE * MAX_MESSAGES] = {0};  // Buffer to store the retrieved messages
            retrieve_messages(message_output, n);  // Retrieve the messages
            sendto(server_fd, message_output, strlen(message_output), 0, (struct sockaddr*)&client_addr, client_addr_len);  // Send the retrieved messages to the client
        } else {  // If the received message is not a "READ" command
            log_message(buffer);  // Log the received message to the log file
        }
    }

    // Close the server socket
    close(server_fd);

    return 0;
}
