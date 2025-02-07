#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_INPUT_LEN 100 // need to change
#define CHANGE 10

// global variables
int R, C;
int start_row = 0;
int start_col = 0;

struct cell
{
    int val;
    int row;
    int col;
    char expression[MAX_INPUT_LEN];
    int status; // to store err
    struct cell **d;
    int count;
};

void create_sheet(struct cell ***sheet)
{
    *sheet = (struct cell **)malloc(R * sizeof(struct cell *));
    for (int i = 0; i < R; i++)
    {
        (*sheet)[i] = (struct cell *)malloc(C * sizeof(struct cell));
        for (int j = 0; j < C; j++)
        {
            (*sheet)[i][j].val = 0;
            (*sheet)[i][j].d = NULL;
            (*sheet)[i][j].count = 0;
            (*sheet)[i][j].status = 0;
            (*sheet)[i][j].row = i;
            (*sheet)[i][j].col = j;
        }
    }
}

int label_to_index(char *s, int *row, int *col)
{
    if (strlen(s) > 6)
        return -1; // Prevent excessive length
    int count1 = 0, count2 = 0;
    int alphabet[3] = {0}, number[3] = {0};
    char *original_s = s;
    while (*s != '\0')
        s++;
    s--;
    while (s >= original_s)
    {
        if ((*s >= 'A') && (*s <= 'Z'))
        {
            count1++;
            if (count2 == 0)
            {
                //  printf("Invalid cell \n");
                return -1; // Invalid cell
            }
            if (count1 > 3)
            {
                // printf("Invalid Input < 1<=C<=18278 \n");
                return -1;
            }
            alphabet[3 - count1] = *s - 'A' + 1;
        }
        else if ((*s >= '0') && (*s <= '9'))
        {
            count2++;
            if (count1 > 0)
            {
                //  printf("Invalid cell \n");
                return -1;
            }
            if (count2 > 3)
            {
                // printf("Invalid Input < 1<=R<=999 \n");

                return -1;
            }
            number[3 - count2] = *s - '0';
        }
        s--;
    }
    *col = alphabet[2] + alphabet[1] * 26 + alphabet[0] * 26 * 26 - 1;
    *row = number[2] + number[1] * 10 + number[0] * 100 - 1;

    return 0;
}

int col_label_to_index(const char *label)
{
    int index = 0;
    while (*label)
    {
        index = index * 26 + (*label - 'A' + 1);
        label++;
    }
    return index - 1;
}

void col_index_to_label(int index, char *label)
{
    char buffer[4] = {0};
    int i = 2;
    do
    {
        buffer[i] = 'A' + (index % 26);
        index = index / 26 - 1;
        i--;
    } while (index >= 0);
    strcpy(label, &buffer[i + 1]);
}

void print_sheet(struct cell ***sheet)
{
    printf("\t");
    for (int col = start_col; col < start_col + 10 && col < C; col++)
    {
        char label[4];
        col_index_to_label(col, label);
        printf("%s\t", label);
    }
    printf("\n");

    for (int row = start_row; row < start_row + 10 && row < R; row++)
    {
        printf("%d\t", row + 1);
        for (int col = start_col; col < start_col + 10 && col < C; col++)
        {
            if ((*sheet)[row][col].status == 1)
            {
                printf("ERR\t");
            }
            else
                printf("%d\t", (*sheet)[row][col].val);
        }
        printf("\n");
    }
}

void add_dependency(struct cell *c, struct cell *dep, int rows, int cols, struct cell ***sheet)
{
    char expr = (*dep).expression;
    char operator;
    char expr1[MAX_INPUT_LEN], expr2[MAX_INPUT_LEN];
    char label1[4], label2[4];
    int row1, row2;
    struct cell dep1, dep2;
    dep1.d = NULL;
    dep2.d = NULL;
    if (sscanf(expr, "%[^*+-/]%c%[^\n]", expr1, &operator, expr2) == 3)
    {

        if (sscanf(expr1, "%[A-Z]%d", label1, &row1) == 2)
        {
            int col1 = col_label_to_index(label1);
            row1--;
            if (col1 < 0 || col1 >= cols || row1 < 0 || row1 >= rows)
            {

                return -1; // Out-of-bounds error
            }
            dep1 = (*sheet)[row1][col1];
        }
        if (sscanf(expr2, "%[A-Z]%d", label2, &row2) == 2)
        {
            int col2 = col_label_to_index(label2);
            row2--;
            if (col2 < 0 || col2 >= cols || row2 < 0 || row2 >= rows)
            {

                return -1; // Out-of-bounds error
            }
            dep2 = (*sheet)[row2][col2];
        }
    }
    if (sscanf(expr, "%[A-Z]%d", label1, &row1) == 2)
    {

        int col1 = col_label_to_index(label1);
        row1--;
        if (col1 < 0 || col1 >= cols || row1 < 0 || row1 >= rows)
        {
            return -1; // Out-of-bounds error
        }
        dep1 = (*sheet)[row1][col1];
    }

    if (dep1.d != NULL)
    {
        int i, j, found = 0;
        // Find the index of `dep` in `dep1->d`
        for (i = 0; i < dep1.count; i++)
        {
            if (dep1.d[i] == dep)
            {
                found = 1;
                break;
            }
        }
        // If `dep` is found, shift elements to the left
        if (found)
        {
            for (j = i; j < dep1.count - 1; j++)
            {
                dep1.d[j] = dep1.d[j + 1];
            }
            // Decrease count
            dep1.count--;
            // Resize the dependency array
            dep1.d = realloc(dep1.d, dep1.count * sizeof(struct cell *));
            if (dep1.count > 0 && !dep1.d)
            {
                printf("Memory reallocation failed!\n");
            }
        }
    }
    if (dep2.d != NULL)
    {
        int i, j, found = 0;
        // Find the index of `dep` in `dep1->d`
        for (i = 0; i < dep2.count; i++)
        {
            if (dep2.d[i] == dep)
            {
                found = 1;
                break;
            }
        }
        // If `dep` is found, shift elements to the left
        if (found)
        {
            for (j = i; j < dep2.count - 1; j++)
            {
                dep2.d[j] = dep2.d[j + 1];
            }
            // Decrease count
            dep2.count--;
            // Resize the dependency array
            dep2.d = realloc(dep2.d, dep2.count * sizeof(struct cell *));
            if (dep2.count > 0 && !dep2.d)
            {
                printf("Memory reallocation failed!\n");
            }
        }
    }

    struct cell **new_d = realloc((*c).d, ((*c).count + 1) * sizeof(struct cell *));
    if (!new_d)
    {
        printf("Memory allocation failed!\n");
        return;
    }
    (*c).count++;
    (*c).d = new_d;
    (*c).d[(*c).count - 1] = dep; // Add the dependency cell to the list
}

int scroll(const char *input)
{
    if (strcmp(input, "q") == 0)
        return 1; // Exit on 'q'

    if (strcmp(input, "w") == 0 && start_row - 10 >= 0)
        start_row -= 10; // Scroll up
    else if (strcmp(input, "w") == 0 && start_row - 10 < 0)
        start_row = 0;
    else if (strcmp(input, "s") == 0 && start_row + 20 <= R)
        start_row += 10; // Scroll down
    else if (strcmp(input, "s") == 0 && (start_row + 10 <= R) && (start_row + 20 > R))
        start_row = R - 9;
    else if (strcmp(input, "a") == 0 && start_col - 10 >= 0)
        start_col -= 10; // Scroll left
    else if (strcmp(input, "a") == 0 && start_col - 10 < 0)
        start_col = 0;
    else if (strcmp(input, "d") == 0 && start_col + 20 <= C)
        start_col += 10; // Scroll right
    else if (strcmp(input, "d") == 0 && (start_col + 10 <= R) && (start_col + 20 > R))
        start_col = C - 9;
    return 0;
}

// r1 r2 c1 c2 for add dependency
int evaluate_expression(char *expr, int rows, int cols, struct cell ***sheet, int *result, int *r1, int *c1, int *r2, int *c2)
{
    int col1, col2, row1, row2;
    *r1 = *c1 = *r2 = *c2 = -1;

    int value1, value2;
    char label1[4], label2[4];

    // VALUE

    if (sscanf(expr, "%d", result) == 1)
        return 0; // NUMBER

    // ARTHIMETIC EXPRESSIONS

    char operator;
    char expr1[MAX_INPUT_LEN], expr2[MAX_INPUT_LEN];
    if (sscanf(expr, "%[^*+-/]%c%[^\n]", expr1, &operator, expr2) == 3)
    {

        if (sscanf(expr1, "%[A-Z]%d", label1, &row1) == 2)
        {
            int col1 = col_label_to_index(label1);
            row1--;
            if (col1 < 0 || col1 >= cols || row1 < 0 || row1 >= rows)
            {

                return -1; // Out-of-bounds error
            }
            *r1 = row1;
            *c1 = col1;
            // add_dependency(&(*sheet)[row1][col1], &(*sheet)[row][col]);
            value1 = (*sheet)[row1][col1].val;
        }
        else if (sscanf(expr1, "%d", &value1) == 1)
        {
            // do nothing
        }
        else
        {
            return -1;
        }
        if (sscanf(expr2, "%[A-Z]%d", label2, &row2) == 2)
        {
            int col2 = col_label_to_index(label2);
            row2--;
            if (col2 < 0 || col2 >= cols || row2 < 0 || row2 >= rows)
            {

                return -1; // Out-of-bounds error
            }
            *r2 = row2;
            *c2 = col2;
            // add_dependency(&(*sheet)[row2][col2], &(*sheet)[row][col]);
            value2 = (*sheet)[row2][col2].val;
        }
        else if (sscanf(expr2, "%d", &value2) == 1)
        {
            // do nothing
        }
        else
        {
            return -1;
        }
        switch (operator)
        {
        case '+':
            *result = value1 + value2;
            return 0;
        case '-':
            *result = value1 - value2;
            return 0;
        case '*':
            *result = value1 * value2;
            return 0;
        case '/':
            if (value2 == 0)
            {

                return -2; // Division by zero error
            }
            *result = value1 / value2;
            break;
        default:
            return -1; // Invalid operator
        }
    }

    // expression is a cell

    if (sscanf(expr, "%[A-Z]%d", label1, &row1) == 2)
    {

        int col1 = col_label_to_index(label1);
        row1--;
        if (col1 < 0 || col1 >= cols || row1 < 0 || row1 >= rows)
        {
            return -1; // Out-of-bounds error
        }
        *result = (*sheet)[row1][col1].val;
        return 0; // Success
    }
    return -1; // Invalid expression
}

void propagate_changes(int rows, int cols, struct cell ***sheet, int row, int col)
{
    // static int visited[999][18278] = {0}; // Large array warning
    // if (visited[row][col]) return;
    // visited[row][col] = 1;
    for (int i = 0; i < (*sheet)[row][col].count; i++)
    {
        int r = (*(*sheet)[row][col].d[i]).row;
        int c = (*(*sheet)[row][col].d[i]).col;
        int result;
        int r1, r2, c1, c2;
        int return_value = evaluate_expression((*sheet)[r][c].expression, rows, cols, sheet, &result, &r1, &c1, &r2, &c2);
        if (return_value == 0)
        {
            (*sheet)[r][c].val = result;
            propagate_changes(rows, cols, sheet, r, c);
        }
        else if (return_value == -2)
        {
            (*sheet)[r][c].status = 1;
            return;
        }
        // (*sheet)[r][c].val = result;
        // propagate_changes(rows, cols, sheet, r, c);
    }
}

int execute_command(const char *input, int rows, int cols, struct cell ***sheet)
{

    if (strcmp(input, "q") == 0 || strcmp(input, "w") == 0 || strcmp(input, "s") == 0 || strcmp(input, "a") == 0 || strcmp(input, "d") == 0)
    {

        return scroll(input);
    }

    char label[7], expr[MAX_INPUT_LEN];
    int col, row, result;
    char col_label[4];
    int row1, col1, row2, col2;

    // SCROLL_TO
    if (sscanf(input, "scroll_to %[A-Z]%d", col_label, &row) == 2)
    {
        int col = col_label_to_index(col_label);

        row--;
        if (col < 0 || col >= C || row < 0 || row >= R)
        {
            return -1; // Out-of-bounds error
        }
        start_row = row;
        start_col = col;
        return 0;
    }

    if (sscanf(input, "%[^=]=%[^\n]", label, expr) == 2)
    {
        if (label_to_index(label, &row, &col) < 0)
        {

            return -1; // Invalid cell
        }
        if (col < 0 || col >= C || row < 0 || row >= R)
        {
            return -1; // Out-of-bounds error
        }
        int return_value = evaluate_expression(expr, rows, cols, sheet, &result, &row1, &col1, &row2, &col2);
        if (return_value == 0)
        {
            (*sheet)[row][col].val = result;
            if (row1 >= 0 && col1 >= 0)
            {
                add_dependency(&(*sheet)[row1][col1], &(*sheet)[row][col], rows , cols, sheet);
            }
            if (row2 >= 0 && col2 >= 0)
            {
                add_dependency(&(*sheet)[row2][col2], &(*sheet)[row][col], rows , cols, sheet);
            }

            strcpy((*sheet)[row][col].expression, expr);
            propagate_changes(rows, cols, sheet, row, col);
            return 0; // Command executed successfully
        }
        else if (return_value == -2)
        {
            if (row1 >= 0 && col1 >= 0)
            {
                add_dependency(&(*sheet)[row1][col1], &(*sheet)[row][col], rows , cols, sheet);
            }
            if (row2 >= 0 && col2 >= 0)
            {
                add_dependency(&(*sheet)[row2][col2], &(*sheet)[row][col], rows , cols, sheet);
            }
            strcpy((*sheet)[row][col].expression, expr);
            (*sheet)[row][col].status = 1;
            return -2; // Division by zero error
        }
        return -1; // Invalid expression
    }
    return -1; // Command not recognized
}

int main(int argc, char *argv[])
{
    char msg[100] = "ok";
    if (argc != 3)
    {
        printf("Usage: %s <No. of rows> <No. of columns>\n", argv[0]);
        return -1;
    }

    R = atoi(argv[1]);
    C = atoi(argv[2]);

    clock_t start_time = clock();

    if (R > 999 || R < 1)
    {
        printf("Invalid Input < 1<=R<=999 \n");
        return -1;
    }
    if (C > 18278 || C < 1)
    {
        printf("Invalid Input < 1<=C<=18278 \n");
        return -1;
    }

    struct cell **sheet;
    create_sheet(&sheet);

    print_sheet(&sheet);

    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time) / CLOCKS_PER_SEC);

    printf("[%.1f] (ok) > ", time_taken);

    char input[MAX_INPUT_LEN];
    while (1)
    {
        if (!fgets(input, MAX_INPUT_LEN, stdin))
            break;
        input[strcspn(input, "\n")] = 0;

        clock_t start = clock();
        int status = execute_command(input, R, C, &sheet);
        clock_t end = clock();
        double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
        print_sheet(&sheet);
        if (status == 0)
        {
            printf("[%0.1f] (ok) > ", time_taken);
        }
        else
        {
            printf("[%0.1f] (error) > ", time_taken);
        }
    }
    // char s[] = "AAA1";
    // int row, col;
    // label_to_index(s, &row, &col);
    // printf("%d %d\n", row, col);
    return 0;
}