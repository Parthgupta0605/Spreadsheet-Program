#include <stdio.h>

int name_to_index(char *s , int c){
    int count1=0,count2=0;
    int arr1[3]={0},arr2[3]={0};
    while(*s!='\0') s++;
    while(*s){
        if ((atoi(*s) >= 65) && (atoi(*s) <= 90)){
            count1++;
            arr1[3-count1]=atoi(*s) - 64;
        }
        else if ((atoi(*s) >= 48) && (atoi(*s) <= 57)){
            count2++;
            arr2[3-count2]=atoi(*s) -48;
        }
        s--;
    }
    int a = arr1[2]+arr1[1]*26+arr1[0]*26*26;
    int b = arr2[2]+arr2[1]*10+arr2[0]*100;
    int result = (b-1)*c + a ;
    return result;
}
int main()
{
    int rows, columns;
    char input ;
    char state[20];
    char first_row[18278][3];
    int i=0; 
    // adjacency list 
    scanf("%d %d", &rows, &columns);
    char exp_list[rows*columns+1][15];    
    char ***adjacency_list = (char**)malloc((rows*columns+1)*sizeof(char**));

    // initialization of first row
    for (char c = 'A' ; c<='Z';c++){
        first_row[i][0] = c;
        i++;
    }
    for (char c1 = 'A' ; c1<='Z';c1++){
        for (char c2 = 'A' ; c2<='Z';c2++){
            first_row[i][0] = c1;
            first_row[i][1] = c2;
            i++;
        }
    }
    for (char c1 = 'A' ; c1<='Z';c1++){
        for (char c2 = 'A' ; c2<='Z';c2++){
            for (char c3 = 'A' ; c3<='Z';c3++){
                first_row[i][0] = c1;
                first_row[i][1] = c2;
                first_row[i][2] = c3;
                i++;
            }
        }
    }

        // initialization of grid
    int grid[rows][columns];
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            grid[i][j] = 0;
        }
    }

    while (1)
    {
        int start_row = 0, start_col = 0;
        int end_row, end_col;
        if (rows >= 10)
            end_row = 9;
        else
            end_row = rows-1;
        if (columns >= 10)
            end_col = 9;
        else
            end_col = columns-1;
       
        // Output 
        printf("  ");
        for (int i = start_col; i<=end_col; i++){
            printf("%s ",first_row[i]);
        }
        printf("\n");
        for(int i = start_row; i<=end_row; i++){
            printf("%d ",(i+1));
            for(int j = start_col; j<=end_col; j++){
                printf("%d ",grid[i][j]);
            }
            printf("\n");
        }
        //printf("[%f] ",);
        printf("(%s) >",state);
        //

        // Input
        //

        // recalculations         
        switch (input){
            case 'w' : 
                if (end_row+1 == rows);
                else {
                    end_row+=1;
                    start_row+=1;
                }
                break;
            case 's' :
                if (start_row == 0);
                else {
                end_row-=1;
                start_row-=1;
                }
                break;
            case 'a' :
                if (end_col+1 == columns);
                else {
                end_col+=1;
                start_col+=1;
                }
                break;
            case 'd' : 
                if (start_col==0);
                else {
                    start_col-=1;
                    end_col-=1;
                }
                break;
            case 'q' : 
                // exit the program                
        } 
        //           
    }
    return 0;
}