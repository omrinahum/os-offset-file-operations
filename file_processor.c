#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>  
#include <unistd.h>
#include <ctype.h> 

#define MAX_LENGTH 256

// Tracks the current size of the data file
int max_offset=0;

int read_exec(FILE* data_file, FILE* results_file, char* s_offset, char* e_offset);
int write_exec(FILE* data_file, char* s_offset, char* text);
void strip_newline(char* str);
void close_all_files(FILE* data_file, FILE* requests_file, FILE* results_file);
bool check_arguments(char key, char* arguments[], int arg_count);
void process_commands(FILE* data_file, FILE* requests_file, FILE* results_file);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <data.txt>, <requests.txt>\n", argv[0]);
        return 1;
    }

    printf("\n=== FILE PROCESSOR ===\n\n");

    // Open data file
    FILE *data_file = fopen(argv[1], "r+");
    if (data_file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", argv[1]);
        return 1;
    }


    char line[MAX_LENGTH];
    max_offset = fread(line, 1, sizeof(line) - 1, data_file);
    if (max_offset < 0) {
        fprintf(stderr, "Error reading file: %s\n", argv[1]);
        fclose(data_file);
        return 1;
    }

    // Open the requests file
    FILE* requests_file = fopen(argv[2], "r");
    if (requests_file == NULL) {
        fprintf(stderr, "Error opening file: requests.txt\n");
        fclose(data_file);
        return 1;
    }

    // Create or truncate the results file
    FILE* results_file= fopen("read_results.txt","w");
    if (results_file == NULL) {
        fprintf(stderr, "Error opening file: read_results.txt\n");
        fclose(data_file);
        fclose(requests_file);
        return 1;
    }

    // Loop procces
    process_commands(data_file, requests_file, results_file);

    close_all_files(data_file, requests_file, results_file);
    
    printf("\n=== DONE ===\n");
    printf("Check 'read_results.txt' for output\n\n");
    
    return 0;    
}


int read_exec (FILE* data_file, FILE* results_file, char* s_offset, char* e_offset){
    int start_offset = atoi(s_offset);
    int end_offset = atoi(e_offset);
    int total_read = end_offset-start_offset + 1;

     // Skip invalid read requests
    if ( start_offset < 0 ||  start_offset > end_offset || end_offset > max_offset  ){
        printf("[READ] Skipped: Invalid offsets (%d to %d)\n", start_offset, end_offset);
        return 0;
    }

    // Pointer to the start position
    if(fseek(data_file,start_offset,SEEK_SET)<0){
        fprintf(stderr, "Error seeking in file\n");
        return -1;
    }

    // Read the requested data
    char buffer[total_read + 1];
    int bytes_read = fread(buffer, 1, sizeof(buffer) - 1, data_file);
    if (bytes_read < 0) {
        fprintf(stderr, "Error reading file\n");
        return -1; 
    }

    buffer[bytes_read] = '\0';

    fprintf(results_file, "%s\n" ,buffer);
    printf("[READ] Read from position %d: \"%s\"\n", start_offset, buffer);
    return 0; 
}



int write_exec (FILE* data_file, char* s_offset, char* text){
    int start_offset= atoi(s_offset);
    int text_size = strlen(text);

    // Skip invalid write requests
    if (start_offset < 0 || start_offset > max_offset ) {
        printf("[WRITE] Skipped: Invalid offset (%d)\n", start_offset);
        return 0;
    }

    // Special case for writing at the exact end of the file
    if (start_offset == max_offset) {
        if (fseek(data_file, start_offset, SEEK_SET) < 0) {
            fprintf(stderr, "Error seeking in file\n");
            return -1;
        }
        fprintf(data_file, "%s", text);
        max_offset += text_size;
        printf("[WRITE] Added \"%s\" at end\n", text);
        return 0;
    }

    // Regular case  writing in the middle of the file
    if(fseek(data_file,start_offset,SEEK_SET)<0){
        fprintf(stderr, "Error seeking in file\n");
        return -1;
    }

    // Save content after write position
    char buffer[max_offset+1];
    int bytes_read = fread(buffer, 1, sizeof(buffer) - 1, data_file);
    if (bytes_read < 0) {
        fprintf(stderr, "Error reading file\n");
        return -1; 
    }
    buffer[bytes_read] = '\0'; 

    // Go back to write position
    if(fseek(data_file,start_offset,SEEK_SET)<0){
        
        return -1;
    }
    
    // Write the new text and existing content
    fprintf(data_file, "%s", text);
    fprintf(data_file, "%s", buffer);

    // Update max_offset 
    max_offset += text_size;
    printf("[WRITE] Inserted \"%s\" at position %d\n", text, start_offset);
    return 0;
}

// Remove newline characters from the end of a string
void strip_newline(char* str) {
    if (str == NULL) return;
    
    int len = strlen(str);
    if (len > 0 && str[len-1] == '\n') {
        str[len-1] = '\0';
    }
    if (len > 1 && str[len-2] == '\r') { 
        str[len-2] = '\0';
    }
}

// Close all open files
void close_all_files(FILE* data_file, FILE* requests_file, FILE* results_file) {
    if (data_file) fclose(data_file);
    if (requests_file) fclose(requests_file);
    if (results_file) fclose(results_file);
}


// Validates command arguments
bool check_arguments(char key, char* arguments[], int arg_count) {
    // Check for minimum number of arguments based on command
    switch (key) {
        case 'R': 
            if (arg_count != 3) {
                fprintf(stderr, "Error: Read command requires 2 offset arguments\n");
                return false;
            }
            break;
        
        case 'W':
            if (arg_count != 3) {
                fprintf(stderr, "Error: Write command requires an offset and text\n");
                return false;
            }
            strip_newline(arguments[2]);
            break;
            
        case 'Q':
            return true;
            
        default:
            fprintf(stderr, "Error: Unknown command '%c'\n", key);
            return false;
    }
    
    // Validate numeric arguments
    if ((key == 'R' || key == 'W') && 
        (!isdigit(arguments[1][0]) && arguments[1][0] != '-')) {
        fprintf(stderr, "Error: Offset must be a number\n");
        return false;
    }
    
    if (key == 'R' && 
        (!isdigit(arguments[2][0]) && arguments[2][0] != '-')) {
        fprintf(stderr, "Error: End offset must be a number\n");
        return false;
    }
    
    return true;
}

// Main command processing loop
void process_commands(FILE* data_file, FILE* requests_file, FILE* results_file) {
    char request_line[MAX_LENGTH];
    while (fgets(request_line, sizeof(request_line), requests_file) != NULL) {
        // Parse the command and arguments
        char* token = strtok(request_line, " ");
        char* arguments[3];
        int i = 0;

        while(token != NULL && i < 3) {
            arguments[i] = token;
            token = strtok(NULL, " ");
            i++;
        }

        char key = arguments[0][0];

        // Skip commands with invalid arguments
        if (!check_arguments(key, arguments, i)) {
            continue;
        }

        // Execute the wanted command with switch case
        switch (key) {
            case 'R':
                if (read_exec(data_file, results_file, arguments[1], arguments[2]) != 0) {
                    fprintf(stderr, "Error in read operation\n");
                    return;  
                }
                break;
            
            case 'W':
                if (write_exec(data_file, arguments[1], arguments[2]) != 0) {
                    fprintf(stderr, "Error in write operation\n");
                    return;  
                }  
                break;

            case 'Q':
                return;  
                
            default:
                printf("Invalid command\n");
                return;  
        }
    }
}