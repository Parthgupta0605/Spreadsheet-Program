#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_INPUT_LEN 100
#define CHANGE 10

//global variables
int R, C;
int start_row = 0; 
int start_col = 0;

struct cell{
    int val;
    struct cell **d;
    int count;
    };

void create_sheet(struct cell ***sheet){
    *sheet = (struct cell **)malloc(R * sizeof(struct cell *));
    for (int i = 0; i < R; i++){
        (*sheet)[i] = (struct cell *)malloc(C * sizeof(struct cell));
        for (int j = 0; j < C; j++){
            (*sheet)[i][j].val = 0;
            (*sheet)[i][j].d = NULL;
            (*sheet)[i][j].count = 0;
        }
    }
}

// int name_to_index(char *s , int c){
//     int count1=0, count2=0;
//     int alphabet[3]={0}, number[3]={0};
//     char *original_s = s;
//     while(*s != '\0') s++;
//     s--;
//     while(s >= original_s){
//         if ((*s >= 'A') && (*s <= 'Z')){
//             count1++;
//             alphabet[3-count1] = *s - 'A' + 1;
//         }
//         else if ((*s >= '0') && (*s <= '9')){
//             count2++;
//             number[3-count2] = *s - '0';
//         }
//         s--;
//     }
//     int a = alphabet[2] + alphabet[1] * 26 + alphabet[0] * 26 * 26;
//     int b = number[2] + number[1] * 10 + number[0] * 100;
    
//     int result = (b-1) * c + a;
//     return result;
// }

void col_index_to_label(int index, char *label) {
    char buffer[4] = {0};
    int i = 2;
    do {
        buffer[i] = 'A' + (index % 26);
        index = index / 26 - 1;
        i--;
    } while (index >= 0);
    strcpy(label, &buffer[i + 1]);
}

void print_sheet(struct cell ***sheet) {
    printf("\t");
    for (int col = start_col; col < start_col + 10 && col<C ; col++) {
        char label[4];
        col_index_to_label(col, label);
        printf("%s\t", label);
    }
    printf("\n");

    for (int row = start_row; row < start_row + 10 && row<R ; row++) {
        printf("%d\t", row + 1);
        for (int col = start_col; col < start_col + 10 && col<C ; col++) {
            printf("%d\t", (*sheet)[row][col].val);
        }
        printf("\n");
    }
}

int scroll(const char *input){
    if (strcmp(input, "q") == 0) return 1; // Exit on 'q'
    
    if (strcmp(input, "w") == 0 && start_row > 0) start_row -= 10; // Scroll up
    else if (strcmp(input, "s") == 0 && start_row + 10 < R) start_row += 10; // Scroll down
    else if (strcmp(input, "a") == 0 && start_col > 0) start_col -= 10; // Scroll left
    else if (strcmp(input, "d") == 0 && start_col + 10 < C) start_col += 10; // Scroll right
    return 0;
}

int execute_command(const char *input, int rows, int cols, struct cell ***sheet) {

    if (strcmp(input, "q") == 0 || strcmp(input, "w") == 0 || strcmp(input, "s") == 0 || strcmp(input, "a") == 0 || strcmp(input, "d") == 0) {

        return scroll(input);
    }

return -1;
}



int main(int argc, char *argv[]){
    char msg[100] = "ok";
    if (argc != 3) {
        printf("Usage: %s <No. of rows> <No. of columns>\n", argv[0]);
        return 1;
}

    R = atoi(argv[1]); 
    C = atoi(argv[2]);

    clock_t start_time = clock();

    if (R>999 || R<1){
        printf("Invalid Input < 1<=R<=999 \n");
        return 1;
    }
    if (C>18278 || C<1){
        printf("Invalid Input < 1<=C<=18278 \n");
        return 1;
    }

    struct cell **sheet;
    create_sheet(&sheet);

    print_sheet(&sheet);

    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)/CLOCKS_PER_SEC);
    
    
    printf("[%.1f] (ok) > ", time_taken);
    
    char input[MAX_INPUT_LEN];
    while(1){
        if (!fgets(input, MAX_INPUT_LEN, stdin)) break;
        input[strcspn(input, "\n")] = 0;
        
        clock_t start = clock();
        int status = execute_command(input, R, C, &sheet);
        clock_t end = clock();
        double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
        print_sheet(&sheet);
        if (status == 0) {
            printf("[%0.1f] (ok) > ", time_taken);
        } else {
            printf("[%0.1f] (error) > ", time_taken);
        }


    }
    return 0;
}