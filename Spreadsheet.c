#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//global variables
int R, C;

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

int name_to_index(char *s , int c){
    int count1=0, count2=0;
    int alphabet[3]={0}, number[3]={0};
    char *original_s = s;
    while(*s != '\0') s++;
    s--;
    while(s >= original_s){
        if ((*s >= 'A') && (*s <= 'Z')){
            count1++;
            alphabet[3-count1] = *s - 'A' + 1;
        }
        else if ((*s >= '0') && (*s <= '9')){
            count2++;
            number[3-count2] = *s - '0';
        }
        s--;
    }
    int a = alphabet[2] + alphabet[1] * 26 + alphabet[0] * 26 * 26;
    int b = number[2] + number[1] * 10 + number[0] * 100;
    
    int result = (b-1) * c + a;
    return result;
}

int main(int argc, char *argv[]){
    char msg[100] = "ok";
        if (argc != 3){
        sprintf(msg, "Usage: %s <No. of rows> <No. of columns>", argv[0]);
    }
    R = atoi(argv[1]); 
    C = atoi(argv[2]);
    clock_t start_time = clock();
    if (R>999 || R<1){
        sprintf(msg, "Invalid Input < 1<=R<=999 >");
    }
    if (C>18278 || C<1){
        sprintf(msg, "Invalid Input < 1<=C<=18278 >");
    }
    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)/CLOCKS_PER_SEC);
    if (strcmp(msg, "ok")) {

        printf("[%.1f] (%s)", time_taken, msg);
    }
    return 0;
}