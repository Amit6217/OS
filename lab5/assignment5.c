#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

#define BUFFER_SIZE 70000

void process2_text_formatter(int read_fd, int write_fd) {
    char in_buf[BUFFER_SIZE];
    char out_buf[BUFFER_SIZE];
    
    ssize_t bytes_read = read(read_fd, in_buf, BUFFER_SIZE - 1);
    if (bytes_read < 0) {
        perror("Process 2 read error");
        exit(EXIT_FAILURE);
    }
    in_buf[bytes_read] = '\0';

    int j = 0;
    int space_pending = 0;
    
    for (int i = 0; i < bytes_read; i++) {
        if (isspace(in_buf[i])) {
            space_pending = 1;
        } else if (ispunct(in_buf[i]) && strchr(".,;:!?", in_buf[i])) {
            // No space before punctuation
            space_pending = 0;
            out_buf[j++] = in_buf[i];
            // Exactly one space after punctuation
            space_pending = 1;
        } else {
            if (space_pending && j > 0 && out_buf[j-1] != ' ') {
                out_buf[j++] = ' ';
            }
            space_pending = 0;
            out_buf[j++] = in_buf[i];
        }
    }
    // Remove trailing space if any
    if (j > 0 && out_buf[j-1] == ' ') {
        j--;
    }
    out_buf[j] = '\0';
    
    if (write(write_fd, out_buf, j) < 0) {
        perror("Process 2 write error");
        exit(EXIT_FAILURE);
    }
}

void process3_case_converter(int read_fd, int write_fd) {
    char in_buf[BUFFER_SIZE];
    
    ssize_t bytes_read = read(read_fd, in_buf, BUFFER_SIZE - 1);
    if (bytes_read < 0) {
        perror("Process 3 read error");
        exit(EXIT_FAILURE);
    }
    in_buf[bytes_read] = '\0';

    int word_count = 0;
    int in_word = 0;
    
    for (int i = 0; i < bytes_read; i++) {
        if (isalpha(in_buf[i])) {
            if (!in_word) {
                in_buf[i] = toupper(in_buf[i]);
                in_word = 1;
                word_count++;
            } else {
                in_buf[i] = tolower(in_buf[i]);
            }
        } else if (isdigit(in_buf[i])) {
            if (!in_word) {
                in_word = 1;
                word_count++;
            }
        } else {
            in_word = 0;
        }
    }
    
    printf("\n--- Final Formatted Text ---\n%s\n----------------------------\n", in_buf);
    
    if (write(write_fd, &word_count, sizeof(int)) < 0) {
        perror("Process 3 write error");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int p1[2], p2[2], p3[2];
    
    if (pipe(p1) < 0 || pipe(p2) < 0 || pipe(p3) < 0) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("Fork failed for Child 1");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) {
        // Process 2 (Child 1) - Text Formatter
        close(p1[1]); // Close unused write end of Pipe 1
        
        close(p2[0]); // Close unused read end of Pipe 2
        
        close(p3[0]); // Close unused Pipe 3
        close(p3[1]);
        
        process2_text_formatter(p1[0], p2[1]);
        
        close(p1[0]);
        close(p2[1]);
        exit(EXIT_SUCCESS);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("Fork failed for Child 2");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) {
        // Process 3 (Child 2) - Case Converter & Word Counter
        close(p1[0]); // Close unused Pipe 1
        close(p1[1]);
        
        close(p2[1]); // Close unused write end of Pipe 2
        
        close(p3[0]); // Close unused read end of Pipe 3
        
        process3_case_converter(p2[0], p3[1]);
        
        close(p2[0]);
        close(p3[1]);
        exit(EXIT_SUCCESS);
    }

    // Process 1 (Parent) - File Reader & Word Count Displayer
    close(p1[0]); // Close unused read end of Pipe 1
    
    close(p2[0]); // Close unused Pipe 2
    close(p2[1]);
    
    close(p3[1]); // Close unused write end of Pipe 3

    char filename[256];
    printf("Enter the input filename: ");
    if (scanf("%255s", filename) != 1) {
        fprintf(stderr, "Error reading filename\n");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read = fread(buffer, 1, BUFFER_SIZE - 1, file);
    buffer[bytes_read] = '\0';
    fclose(file);

    // Send to Process 2
    if (write(p1[1], buffer, bytes_read) < 0) {
        perror("Process 1 write error");
        exit(EXIT_FAILURE);
    }
    close(p1[1]); // Close to send EOF/finish

    // Read word count from Process 3
    int total_words = 0;
    if (read(p3[0], &total_words, sizeof(int)) < 0) {
        perror("Process 1 read error");
        exit(EXIT_FAILURE);
    }
    close(p3[0]);

    // Wait for children
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    printf("Total number of words: %d\n", total_words);

    return 0;
}
