#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>
#include "avl.c"
#include "cell.h"
#include "stack.c"
#define MAX_INPUT_LEN 35

// global variables
int flag = 1; //Enabled output
int R, C;
int start_row = 0;
int start_col = 0;

void create_sheet(struct cell ***sheet)
{
    // Allocate all memory for the entire sheet in a single contiguous block.
    struct cell *data = (struct cell *)calloc(R * C, sizeof(struct cell));
    if (!data)  // Check if allocation failed
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for row pointers (array of struct cell pointers).
    *sheet = (struct cell **)malloc(R * sizeof(struct cell *));
    if (!*sheet)  // Check if allocation failed
    {
        perror("Memory allocation failed");
        free(data); // Free previously allocated block before exiting
        exit(EXIT_FAILURE);
    }

    // Assign row pointers to corresponding positions in the contiguous memory block.
    for (int i = 0, index = 0; i < R; i++)
    {
        (*sheet)[i] = &data[index]; // Point each row to the correct starting position
        index += C; // Move to the next row in the contiguous block
    }
}

int label_to_index(char *s, int *row, int *col)
{
    if (strlen(s) > 6)
        return -1; // Prevent excessive length to ensure valid cell format

    int count1 = 0, count2 = 0; // Counters for letters and numbers
    int alphabet[3] = {0, 0, 0}; // Stores column letters (up to 3 characters)
    int number[3] = {-1, -1, -1}; // Stores row digits (up to 3 characters)

    char *original_s = s; // Store original pointer location

    // Move pointer to the end of the string
    while (*s != '\0')
        s++;
    s--; // Move back to the last character

    // Traverse the string in reverse
    while (s >= original_s)
    {
        if ((*s >= 'A') && (*s <= 'Z')) // If character is an uppercase letter
        {
            count1++;
            if (count2 == 0) // Letters should be before numbers
            {
                return -1; // Invalid format
            }
            if (count1 > 3) // Maximum 3 letters allowed
            {
                return -1;
            }
            alphabet[3 - count1] = *s - 'A' + 1; // Store letter index
        }
        else if ((*s >= '0') && (*s <= '9')) // If character is a digit
        {
            count2++;
            if (count1 > 0) // Numbers should be at the end
            {
                return -1;
            }
            if (count2 > 3) // Maximum 3 digits allowed
            {
                return -1;
            }
            number[3 - count2] = *s - '0'; // Store number
        }
        else
        {
            return -1; // Invalid character
        }
        s--; // Move to previous character 
    }

    // Check for invalid number formats (e.g., empty or leading zeros)
    if ((number[0] == -1 && number[1] == -1 && number[2] == 0) || 
        (number[0] == -1 && number[1] == 0) || 
        (number[0] == 0))
    {
        return -1; // Number has leading zeroes
    }
    

    // Normalize missing digits
    if (number[0] == -1) number[0] = 0;
    if (number[1] == -1) number[1] = 0;

    // Convert column label to zero-based index
    *col = alphabet[2] + alphabet[1] * 26 + alphabet[0] * 26 * 26 - 1;

    // Convert row digits to zero-based index
    *row = number[2] + number[1] * 10 + number[0] * 100 - 1;

    return 0; // Successful conversion
}

int col_label_to_index(const char *label)
{
    int index = 0;

    // Convert column label (e.g., "A", "Z", "AA") to zero-based index
    while (*label)
    {
        if(*label < 'A' || *label > 'Z')
            return -1; 
        index = index * 26 + (*label - 'A' + 1);
        label++;
    }

    return index - 1; // Adjust to zero-based indexing
}

void col_index_to_label(int index, char *label)
{
    char buffer[4] = {0}; // Temporary buffer for the column label (max 3 letters)
    int i = 2; // Start from the rightmost position

    do
    {
        buffer[i] = 'A' + (index % 26); // Compute the current letter
        index = index / 26 - 1; // Move to the next letter in the sequence
        i--;
    } while (index >= 0);

    // Copy the computed label from the buffer to the output string
    strcpy(label, &buffer[i + 1]);
}

void print_sheet(struct cell ***sheet)
{
    // Print column labels (A, B, C, ...), starting from `start_col`
    printf("\t");
    for (int col = start_col; col < start_col + 10 && col < C; col++)
    {
        char label[4]; // Buffer to store column label
        col_index_to_label(col, label); // Convert column index to label
        printf("%s\t", label);
    }
    printf("\n");

    // Iterate through rows starting from `start_row`
    for (int row = start_row; row < start_row + 10 && row < R; row++)
    {
        // Print row number (1-based index)
        printf("%d\t", row + 1);

        // Iterate through columns and print cell values
        for (int col = start_col; col < start_col + 10 && col < C; col++)
        {
            if ((*sheet)[row][col].status == 1) // If cell has an error
            {
                printf("ERR\t"); // Print "ERR" for error status
            }
            else
            {
                printf("%d\t", (*sheet)[row][col].val); // Print cell value
            }
        }
        printf("\n"); // Move to next row
    }
}

void add_dependency(struct cell *c, struct cell *dep, struct cell ***sheet)
{
    // Insert `dep` into the list of dependencies of cell `c`
    c->dependencies = insert(c->dependencies, dep, sheet, C);
}

void add_dependent(struct cell *c, struct cell *dep)
{
    // Add `dep` to the list of dependents of cell `c`
    push_dependent(c, dep);
}

int scroll(const char *input)
{
    // Scroll up
    if (strcmp(input, "w") == 0 && start_row - 10 >= 0)
        start_row -= 10;
    else if (strcmp(input, "w") == 0 && start_row - 10 < 0)
        start_row = 0; // Ensure we don’t scroll beyond the first row

    // Scroll down
    else if (strcmp(input, "s") == 0 && start_row + 20 <= R - 1)
        start_row += 10;
    else if (strcmp(input, "s") == 0 && (start_row + 20 >= R))
        start_row = R - 10; // Ensure we don’t scroll beyond the last row

    // Scroll left
    else if (strcmp(input, "a") == 0 && start_col - 10 >= 0)
        start_col -= 10;
    else if (strcmp(input, "a") == 0 && start_col - 10 < 0)
        start_col = 0; // Ensure we don’t scroll beyond the first column

    // Scroll right
    else if (strcmp(input, "d") == 0 && start_col + 20 <= C - 1)
        start_col += 10;
    else if (strcmp(input, "d") == 0 && (start_col + 20 >= C))
        start_col = C - 10; // Ensure we don’t scroll beyond the last column

    return 0;
}

void sleep_seconds(int seconds)
{
    // Store the current time as the start time
    time_t start_time = time(NULL);

    while (difftime(time(NULL), start_time) < seconds)
    {
        // Wait for the time equals seconds
    }
}

void delete_dependencies(struct cell *cell1, int row, int col, struct cell ***sheet)
{
    // Iterate through all dependents of `cell1`
    while (cell1->dependents != NULL)
    {
        // Get the AVL tree of dependencies for the dependent cell
        struct avl_node *dependencies = cell1->dependents->cell->dependencies;

        // Remove `cell1` from the dependencies of the dependent cell
        cell1->dependents->cell->dependencies = delete_node(dependencies, row, col, sheet, C);

        // Remove the dependent relationship
        pop_dependent(cell1);
    }
}

bool dfs(struct cell *current, struct cell *target, bool *visited, int current_row, int current_col, struct cell ***sheet)
{
    // If we reach the target cell, a cycle is detected
    if (current == target)
    {
        return true;
    }
    // Check if the target cell is directly in the dependency tree of `current`
    if (find(current->dependencies, calculate_row(target, sheet, C), calculate_col(target, sheet, C), sheet, C) != NULL)
    {
        return true;
    }
    // If the current cell hasn't been visited yet
    if (!visited[current_row * C + current_col])
    {
        // Mark the current cell as visited
        visited[current_row * C + current_col] = true;

        // Iterate through all dependencies of `current`
        struct avl_node *dependencies = current->dependencies;
        while (dependencies != NULL)
        {
            struct cell *dep_cell = dependencies->cell;

            // Compute row and column indices of the dependent cell
            int dep_row = (dep_cell - &(*sheet)[0][0]) / C;
            int dep_col = (dep_cell - &(*sheet)[0][0]) % C;

            // Perform DFS on dependent cells
            if (dfs(dep_cell, target, visited, dep_row, dep_col, sheet))
            {
                return true; // Cycle detected
            }

            // Also check the left subtree of AVL tree
            if (dependencies->left != NULL && dfs(dependencies->left->cell, target, visited, dep_row, dep_col, sheet))
            {
                return true; // Cycle detected
            }
            // Move to the right subtree in AVL tree
            dependencies = dependencies->right;
        }
    }
    return false;
}

bool check_loop(struct cell *start, struct cell *target, int start_row, int start_col, int target_row, int target_col, struct cell ***sheet)
{
    // Allocate a boolean array to track visited cells (initialized to false)
    bool *visited = (bool *)calloc(R * C, sizeof(bool));

    // Perform DFS to check for cycles
    bool result = dfs(start, target, visited, start_row, start_col, sheet);

    // Free allocated memory
    free(visited);

    return result;
}

bool dfs_range(struct cell *current, bool *visited, int row1, int col1, int row2, int col2, int current_row, int current_col, struct cell ***sheet)
{
    // If the current cell falls within the given range, return true
    if (current_row >= row1 && current_row <= row2 && current_col >= col1 && current_col <= col2)
    {
        return true;
    }

    // If the current cell hasn't been visited yet
    if (!visited[current_row * C + current_col])
    {
        // Mark the current cell as visited
        visited[current_row * C + current_col] = true;

        // Iterate through all dependencies of `current`
        struct avl_node *dependencies = current->dependencies;
        while (dependencies != NULL)
        {
            struct cell *dep_cell = dependencies->cell;

            // Compute row and column indices of the dependent cell
            int dep_row = (dep_cell - &(*sheet)[0][0]) / C;
            int dep_col = (dep_cell - &(*sheet)[0][0]) % C;

            // Perform DFS on dependent cells
            if (dfs_range(dep_cell, visited, row1, col1, row2, col2, dep_row, dep_col, sheet))
            {
                return true; // Dependency found in range
            }

            // Also check the left subtree of AVL tree
            if (dependencies->left != NULL && dfs_range(dependencies->left->cell, visited, row1, col1, row2, col2, dep_row, dep_col, sheet))
            {
                return true;
            }

            // Move to the right subtree in AVL tree
            dependencies = dependencies->right;
        }
    }
    return false;
}

bool check_loop_range(struct cell *start, int row1, int col1, int row2, int col2, int start_row, int start_col, struct cell ***sheet)
{
    // Allocate a boolean array to track visited cells (initialized to false)
    bool *visited = (bool *)calloc(R * C, sizeof(bool));

    // Perform DFS to check for dependencies in the given range
    bool result = dfs_range(start, visited, row1, col1, row2, col2, start_row, start_col, sheet);

    // Free allocated memory
    free(visited);

    return result;
}

void topological_sort_util(struct cell *cell, bool *visited, struct cell ***sheet, struct stack_node **stack)
{
    // Compute row and column indices of the current cell
    int row = (cell - &(*sheet)[0][0]) / C;
    int col = (cell - &(*sheet)[0][0]) % C;

    // If the cell has not been visited, process it
    if (!visited[row * C + col])
    {
        // Mark the cell as visited
        visited[row * C + col] = true;

        // Recursively visit all dependencies (cells that this cell depends on)
        struct avl_node *dependencies = cell->dependencies;
        while (dependencies != NULL)
        {
            struct cell *dep_cell = dependencies->cell;
            
            // Recursively call topological sort for the dependent cell
            topological_sort_util(dep_cell, visited, sheet, stack);
            
            // Also check the left subtree of AVL tree (if exists)
            if (dependencies->left != NULL)
            {
                topological_sort_util(dependencies->left->cell, visited, sheet, stack);
            }

            // Move to the right subtree in AVL tree
            dependencies = dependencies->right;
        }

        // Push the current cell onto the stack after processing its dependencies
        push(stack, cell);
    }
}

void topological_sort_from_cell(struct cell *start_cell, struct cell ***sheet, struct stack_node **stack)
{
    // Allocate a boolean array to track visited cells (initialized to false)
    bool *visited = (bool *)calloc(R * C, sizeof(bool));

    // Perform topological sorting starting from the given cell
    topological_sort_util(start_cell, visited, sheet, stack);

    // Free allocated memory after sorting
    free(visited);
}

int evaluate_expression(char *expr, int rows, int cols, struct cell ***sheet, int *result, int *row, int *col, int call_value)
{
    
    int count_status = 0;
    int col1 = -1;
    int row1 = -1;
    int col2 = -1;
    int row2 = -1;
    char temp[MAX_INPUT_LEN] = "";
    int value1, value2;
    char label1[4], label2[4];

    // Expression is a integer

    if (sscanf(expr, "%d%s", result, temp) == 1)
    {
        if (call_value == 1)
        {
            struct cell *current = &(*sheet)[*row][*col];
            delete_dependencies(current, *row, *col, sheet);
        }
        return 0;
    } 

    // ARTHIMETIC EXPRESSIONS

    char operator;
    char expr1[MAX_INPUT_LEN], expr2[MAX_INPUT_LEN];

    //If first part of the expression is a integer
    if (sscanf(expr, "%d%c%[^\n]", &value1, &operator, expr2) == 3)
    {
        //If second part of the expression is a cell
        if (sscanf(expr2, "%[A-Z]%d%s", label2, &row2, temp) == 2)
        {
            if (expr2[strlen(label2)] == '0')
            {
                return -1; // Invalid cell
            }
            col2 = col_label_to_index(label2); 
            row2--; //To convert 1-based index to 0-based index
            if (col2 < 0 || col2 >= cols || row2 < 0 || row2 >= rows)
            {

                return -1; // Cell is Out-of-bounds error
            }
            if (check_loop(&(*sheet)[*row][*col], &(*sheet)[row2][col2], *row, *col, row2, col2, sheet))
            {
                return -4; // Circular dependency detected
            }

            if ((*sheet)[row2][col2].status == 1)
            {
                count_status++; //For return -2 error
            }
            value2 = (*sheet)[row2][col2].val;
        }
        else if (sscanf(expr2, "%d%s", &value2, temp) == 1) //If second part of the expression is a integer
        {
            // do nothing and make value2 = the integer value of the second part of the expression
        }
        else
        {
            return -1; //Invalid expression
        }

        //If new expression is assigned to cell, new dependencies and dependents are added to the cell and old ones are deleted
        if (call_value == 1 ) 
        {
            struct cell *current = &(*sheet)[*row][*col];
            delete_dependencies(current, *row, *col, sheet);
            if (col2 >= 0 && row2 >= 0)
            {
                add_dependency(&(*sheet)[row2][col2], &(*sheet)[*row][*col], sheet);
                add_dependent(&(*sheet)[*row][*col], &(*sheet)[row2][col2]);
            }            
        }
        //If any of the dependents have ERR , return -2
        if (count_status > 0)
        { 
            return -2;
        }

        //Perform the operation based on the operator
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
            return 0;
        default:
            return -1; // Invalid operator
        }
    }
    else if (sscanf(expr, "%[^*+-/]%c%[^\n]", expr1, &operator, expr2) == 3) //If first part of the expression is a cell
    {
        if (sscanf(expr1, "%[A-Z]%d%s", label1, &row1, temp) == 2) 
        {
            if (expr1[strlen(label1)] == '0')
            {

                return -1; // Invalid cell
            }
            col1 = col_label_to_index(label1);
            row1--; //To convert 1-based index to 0-based index
            if (col1 < 0 || col1 >= cols || row1 < 0 || row1 >= rows)
            {

                return -1; // Out-of-bounds error
            }
            if (check_loop(&(*sheet)[*row][*col], &(*sheet)[row1][col1], *row, *col, row1, col1, sheet))
            {
                return -4; // Circular dependency detected
            }

            //To check If any of the dependents have ERR and return -2
            if ((*sheet)[row1][col1].status == 1)
            {
                count_status++;
            }
            value1 = (*sheet)[row1][col1].val;
        }
        else if (sscanf(expr1, "%d%s", &value1, temp) == 1)
        {
            // do nothing 
        }
        else
        {
            return -1; //Invalid first part of expression
        }

        if (sscanf(expr2, "%[A-Z]%d%s", label2, &row2, temp) == 2) //If second part of the expression is a cell
        {
            if (expr2[strlen(label2)] == '0')
            {
                return -1; // Invalid cell
            }
            col2 = col_label_to_index(label2);
            row2--; //To convert 1-based index to 0-based index
            if (col2 < 0 || col2 >= cols || row2 < 0 || row2 >= rows)
            {

                return -1; // Out-of-bounds error
            }
            if (check_loop(&(*sheet)[*row][*col], &(*sheet)[row2][col2], *row, *col, row2, col2, sheet))
            {
                return -4; // Circular dependency detected
            }

            //To check If any of the dependents have ERR and return -2
            if ((*sheet)[row2][col2].status == 1)
            {
                count_status++;
            }
            value2 = (*sheet)[row2][col2].val; //To get the value of the second part of the expression
        }
        else if (sscanf(expr2, "%d%s", &value2, temp) == 1)
        {
            // do nothing and make value2 = the integer value of the second part of the expression
        }
        else
        {
            return -1; //Invalid second part of expression
        }

        //If new expression is assigned to cell, new dependencies and dependents are added to the cell and old ones are deleted
        if (call_value == 1)
        {
            struct cell *current = &(*sheet)[*row][*col];
            delete_dependencies(current, *row, *col, sheet);
            if (col1 >= 0 && row1 >= 0)
        {
            add_dependency(&(*sheet)[row1][col1], &(*sheet)[*row][*col], sheet);
            add_dependent(&(*sheet)[*row][*col], &(*sheet)[row1][col1]);
        }
        if (col2 >= 0 && row2 >= 0 && (col2!=col1 || row2!=row1)) //To check if the cell is not the same as the first part of the expression
        {
            add_dependency(&(*sheet)[row2][col2], &(*sheet)[*row][*col], sheet);
            add_dependent(&(*sheet)[*row][*col], &(*sheet)[row2][col2]);
        }
        }
        
        //If any of the dependents have ERR , return -2
        if (count_status > 0)
        { 
            return -2;
        }

        //Perform the operation based on the operator
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
            return 0;
        default:
            return -1; // Invalid operator
        }
    }

    char func[10];

    // FUNCTION EXPRESSIONS

    if (sscanf(expr, "%9[A-Z](%[A-Z]%d:%[A-Z]%d)%s", func, label1, &row1, label2, &row2, temp) == 5)
    {
        if (expr[strlen(func) + strlen(label1) + 1] == '0')
        {
            return -1; // Invalid expression
        }

        int row1_begin = row1;
        int row2_begin = row2;
        int len_row1 = 0;
        int len_row2 = 0;

        //To get the length of the row number to check if it is a valid input or cell
        while (row1 > 0)
        {
            row1 = row1 / 10;
            len_row1++;
        }
        while (row2 > 0)
        {
            row2 = row2 / 10;
            len_row2++;
        }
        row1 = row1_begin;
        row2 = row2_begin;
        //To check if the cell is valid and if the expression is valid
        if (expr[strlen(func) + strlen(label1) + 1 + len_row1 + 1 + strlen(label2)] == '0')
        {
            return -1; // Invalid cell
        }
        if (expr[strlen(func) + strlen(label1) + 1 + len_row1 + 1 + strlen(label2) + len_row2] != ')')
        {
            return -1; // Invalid cell
        }
        col1 = col_label_to_index(label1);
        col2 = col_label_to_index(label2);
        row1--; //To convert 1-based index to 0-based index
        row2--;
        if (col1 < 0 || col1 >= cols || row1 < 0 || row1 >= rows || col2 < 0 || col2 >= cols || row2 < 0 || row2 >= rows || row2 < row1 || col2 < col1)
        {
            return -1; // Out-of-bounds error
        }
        if (check_loop_range(&(*sheet)[*row][*col], row1, col1, row2, col2, *row, *col, sheet))
        {
            return -4; // Circular dependency detected
        }

        //SUM FUNCTION

        if (strcmp(func, "SUM") == 0)
        {
            *result = 0;

            //If new expression is assigned to cell, old dependencies and dependents are deleted.
            if (call_value == 1)
            {
                struct cell *current = &(*sheet)[*row][*col];
                delete_dependencies(current, *row, *col, sheet);
            }
            //To get the sum of the cells in the range
            for (int i = row1; i <= row2; i++)
            {
                for (int j = col1; j <= col2; j++)
                {
                    if ((*sheet)[i][j].status == 1)
                    {
                        count_status++;
                    }
                    *result += (*sheet)[i][j].val;
                    //If new expression is assigned to cell, new dependencies and dependents are added to the cell.
                    if(call_value==1){
                        add_dependency(&(*sheet)[i][j], &(*sheet)[*row][*col], sheet);
                        add_dependent(&(*sheet)[*row][*col], &(*sheet)[i][j]);
                    }
                }
            }
            //If any of the dependents have ERR , return -2
            if (count_status > 0)
            { 
                return -2;
            }
            return 0;
        }

        //AVG FUNCTION

        if (strcmp(func, "AVG") == 0)
        {
            *result = 0;
            int count = 0;

            //If new expression is assigned to cell, old dependencies and dependents are deleted.
            if (call_value == 1)
            {
                struct cell *current = &(*sheet)[*row][*col];
                delete_dependencies(current, *row, *col, sheet);
            }

            //To get the sum and count of the cells in the range
            for (int i = row1; i <= row2; i++)
            {
                for (int j = col1; j <= col2; j++)
                {
                    if ((*sheet)[i][j].status == 1)
                    {
                        count_status++;
                    }
                    *result += (*sheet)[i][j].val;
                    count++;
                    //If new expression is assigned to cell, new dependencies and dependents are added to the cell.
                    if(call_value==1){
                        add_dependency(&(*sheet)[i][j], &(*sheet)[*row][*col], sheet);
                        add_dependent(&(*sheet)[*row][*col], &(*sheet)[i][j]);
                    }
                }
            }
            //calculating average
            *result /= count;

            //If any of the dependents have ERR , return -2
            if (count_status > 0)
            { 
                return -2;
            }
            return 0;
        }


        //MAX FUNCTION
        if (strcmp(func, "MAX") == 0)
        {
            //Initialise result to the minimum value
            *result = INT_MIN;
            if (call_value == 1)
            {
                struct cell *current = &(*sheet)[*row][*col];
                delete_dependencies(current, *row, *col, sheet);
            }
            for (int i = row1; i <= row2; i++)
            {
                for (int j = col1; j <= col2; j++)
                {
                    if(call_value==1){
                        add_dependency(&(*sheet)[i][j], &(*sheet)[*row][*col], sheet);
                        add_dependent(&(*sheet)[*row][*col], &(*sheet)[i][j]);
                    }

                    if ((*sheet)[i][j].status == 1)
                    {
                        count_status++;
                    }

                    //If value of the cell is greater than the result, update the result
                    if ((*sheet)[i][j].val > *result)
                    {
                        *result = (*sheet)[i][j].val;
                    }
                }
            }
            if (count_status > 0)
            { // check if any of its dependents have error
                return -2;
            }
            return 0;
        }

        //MIN FUNCTION
        if (strcmp(func, "MIN") == 0)
        {
            *result = INT_MAX;
            if (call_value == 1)
            {
                struct cell *current = &(*sheet)[*row][*col];
                delete_dependencies(current, *row, *col, sheet);
            }
            for (int i = row1; i <= row2; i++)
            {
                for (int j = col1; j <= col2; j++)
                {
                    if ((*sheet)[i][j].status == 1)
                    {
                        count_status++;
                    }

                    if(call_value==1){
                        add_dependency(&(*sheet)[i][j], &(*sheet)[*row][*col], sheet);
                        add_dependent(&(*sheet)[*row][*col], &(*sheet)[i][j]);
                    }

                    //If value of the cell is less than the result, update the result
                    if ((*sheet)[i][j].val < *result)
                    {
                        *result = (*sheet)[i][j].val;
                    }
                }
            }

            //If any of the dependents have ERR , return -2
            if (count_status > 0)
            { 
                return -2;
            }
            return 0;
        }

        //STDEV FUNCTION
        if (strcmp(func, "STDEV") == 0)
        {

            int sum = 0;
            int count = 0;
            if (call_value == 1)
            {
                struct cell *current = &(*sheet)[*row][*col];
                delete_dependencies(current, *row, *col, sheet);
            }
            for (int i = row1; i <= row2; i++)
            {
                for (int j = col1; j <= col2; j++)
                {
                    if ((*sheet)[i][j].status == 1)
                    {
                        count_status++;
                    }

                    if(call_value==1){
                        add_dependency(&(*sheet)[i][j], &(*sheet)[*row][*col], sheet);
                        add_dependent(&(*sheet)[*row][*col], &(*sheet)[i][j]);
                    }

                    sum += (*sheet)[i][j].val;
                    count++;
                }
            }
            //Calculating mean 
            int mean = sum / count;
            double variance = 0.0;

            //Calculating variance
            for (int i = row1; i <= row2; i++)
            {
                for (int j = col1; j <= col2; j++)
                {
                    variance += ((*sheet)[i][j].val - mean) * ((*sheet)[i][j].val - mean);
                }
            }

            //Calculating standard deviation
            variance /= count;
            *result = (int)round(sqrt(variance));

            //If any of the dependents have ERR , return -2
            if (count_status > 0)
            { 
                return -2;
            }
            return 0;
        }
        return -1; // Invalid function
    }

    // SLEEP FUNCTION 

    // expression is a integer in sleep
    if (sscanf(expr, "SLEEP(%d%s)", result, temp) == 2 && temp[0] != ')')
    {
        return -1;
    }
    else if (sscanf(expr, "SLEEP(%d)%s", result, temp) == 1)
    {
        if (*result < 0)
        {
            return -1;
        }
        sleep_seconds(*result);
        return 0;
    }

    // expression is a cell in sleep
    if (sscanf(expr, "SLEEP(%[A-Z]%d%s)", label1, &row1, temp) == 3 && temp[0] != ')')
    {
        return -1;
    }
    else if (sscanf(expr, "SLEEP(%[A-Z]%d)%s", label1, &row1, temp) == 2)
    {
        if (expr[strlen("SLEEP") + strlen(label1) + 1] == '0')
        {
            return -1; // Invalid cell
        }
        int col1 = col_label_to_index(label1);
        row1--;
        if (col1 < 0 || col1 >= cols || row1 < 0 || row1 >= rows)
        {
            return -1; // Out-of-bounds error
        }
        if (check_loop(&(*sheet)[*row][*col], &(*sheet)[row1][col1], *row, *col, row1, col1, sheet))
        {
            return -4; // Circular dependency detected
        }
        if ((*sheet)[row1][col1].status == 1)
        {
            count_status++;
        }
        *result = (*sheet)[row1][col1].val;
        if (call_value == 1)
        {
            struct cell *current = &(*sheet)[*row][*col];
            delete_dependencies(current, *row, *col, sheet);
            add_dependency(&(*sheet)[row1][col1], &(*sheet)[*row][*col], sheet);
            add_dependent(&(*sheet)[*row][*col], &(*sheet)[row1][col1]);
        }

      

        sleep_seconds(*result);
        if (count_status > 0)
        { // check if any of its dependents have error
            return -2;
        }
        return 0;
    }

    // expression is a cell

    if (sscanf(expr, "%[A-Z]%d%s", label1, &row1, temp) == 2)
    {
        if (expr[strlen(label1)] == '0')
        {
            return -1; // Invalid cell
        }
        int col1 = col_label_to_index(label1);
        row1--;
        if (col1 < 0 || col1 >= cols || row1 < 0 || row1 >= rows)
        {
            return -1; // Out-of-bounds error
        }
        if (check_loop(&(*sheet)[*row][*col], &(*sheet)[row1][col1], *row, *col, row1, col1, sheet))
        {
            return -4; // Circular dependency detected
        }
        if ((*sheet)[row1][col1].status == 1)
        {
            count_status++;
        }
        *result = (*sheet)[row1][col1].val;
        if (call_value == 1)
        {
            struct cell *current = &(*sheet)[*row][*col];
            delete_dependencies(current, *row, *col, sheet);
            add_dependency(&(*sheet)[row1][col1], &(*sheet)[*row][*col], sheet);
            add_dependent(&(*sheet)[*row][*col], &(*sheet)[row1][col1]);
        }
       
        if (count_status > 0)
        { // check if any of its dependents have error
            return -2;
        }
        return 0; // Success
    }

    return -1; // Invalid expression
}

int execute_command(const char *input, int rows, int cols, struct cell ***sheet)
{
    // If the input command is 'q', free allocated memory and exit
    if (strcmp(input, "q") == 0)
    {
        free(*sheet[0]);
        free(*sheet);
        return 1; // Exit command
    }

    // Handle scrolling commands ('w', 's', 'a', 'd')
    if (strcmp(input, "w") == 0 || strcmp(input, "s") == 0 || strcmp(input, "a") == 0 || strcmp(input, "d") == 0)
    {
        return scroll(input);
    }

    char label[7], expr[MAX_INPUT_LEN];
    int col, row, result;
    char col_label[4];

    // Handle the 'scroll_to' command
    if (sscanf(input, "scroll_to %[A-Z]%d", col_label, &row) == 2)
    {
        if (input[strlen("scroll_to") + strlen(col_label) + 1] == '0')
        {
            return -1; // Invalid cell
        }
        int col = col_label_to_index(col_label);
        row--;
        
        // Check if the target position is within bounds
        if (col < 0 || col >= C || row < 0 || row >= R)
        {
            return -1; // Out-of-bounds error
        }
        
        // Update scrolling position
        start_row = row;
        start_col = col;
        return 0;
    }

    // Handle commands to disable/enable output
    if (strcmp(input, "disable_output") == 0)
    {
        flag = 0;
        return 0;
    }
    if (strcmp(input, "enable_output") == 0)
    {
        flag = 1;
        return 0;
    }

    // Handle cell assignment (e.g., "A1=5+3")
    if (sscanf(input, "%[^=]=%[^\n]", label, expr) == 2)
    {
        // Convert label (e.g., "A1") to row and column indices
        if (label_to_index(label, &row, &col) == -1)
        {
            return -1; // Invalid cell expression
        }
        // Check for out-of-bounds access
        if (col < 0 || col >= C || row < 0 || row >= R)
        {
            return -1; // Out-of-bounds error
        }

        // Evaluate the expression and store the result in the cell
        int return_value = evaluate_expression(expr, rows, cols, sheet, &result, &row, &col, 1);
        if (return_value == 0)
        {
            (*sheet)[row][col].val = result;
            strcpy((*sheet)[row][col].expression, expr);
            (*sheet)[row][col].status = 0;

            // Perform topological sorting for dependent cells
            struct stack_node *stack = NULL;
            struct cell *start_cell = &(*sheet)[row][col];
            topological_sort_from_cell(start_cell, sheet, &stack);
            pop(&stack);

            // Propagate changes to dependent cells
            while (stack != NULL)
            {
                struct cell *cell = pop(&stack);
                int r = (cell - &(*sheet)[0][0]) / C;
                int c = (cell - &(*sheet)[0][0]) % C;
                int res;
                int ret_val = evaluate_expression(cell->expression, rows, cols, sheet, &res, &r, &c, 1);
                if (ret_val == 0)
                {
                    cell->val = res;
                    cell->status = 0;
                }
                else if (ret_val == -2)
                {
                    cell->status = 1;
                }
            }
            return 0; // Command executed successfully
        }
        else if (return_value == -2)
        {
            // Handle division by zero error
            strcpy((*sheet)[row][col].expression, expr);
            (*sheet)[row][col].status = 1;
            
            struct stack_node *stack = NULL;
            struct cell *start_cell = &(*sheet)[row][col];
            topological_sort_from_cell(start_cell, sheet, &stack);

            // Propagate changes even if an error occurs
            while (stack != NULL)
            {
                struct cell *cell = pop(&stack);
                int r = (cell - &(*sheet)[0][0]) / C;
                int c = (cell - &(*sheet)[0][0]) % C;
                int res;
                int ret_val = evaluate_expression(cell->expression, rows, cols, sheet, &res, &r, &c, 1);
                if (ret_val == 0)
                {
                    cell->val = res;
                    cell->status = 0;
                }
                else if (ret_val == -2)
                {
                    cell->status = 1;
                }
            }
            return -2; // Division by zero error
        }
        else if (return_value == -4)
        {
            return -4; // Circular dependency detected
        }
        return -1; // Invalid expression
    }
    return -1; // Command not recognized
}

#ifndef TEST_MODE
int main(int argc, char *argv[])
{
    // char msg[100] = "ok";
    if (argc != 3)
    {
        printf("Usage: %s <No. of rows> <No. of columns>\n", argv[0]);
        return -1;
    }

    R = atoi(argv[1]);
    C = atoi(argv[2]);

    // clock_t start_time = clock();
    time_t start_time = time(NULL);
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

    // clock_t end_time = clock();
    time_t end_time = time(NULL);
    double time_taken = difftime(end_time, start_time);

    //  double time_taken = ((double)(end_time - start_time) / CLOCKS_PER_SEC);

    printf("[%.2f] (ok) > ", time_taken);

    char input[MAX_INPUT_LEN];
    while (1)
    {
        if (!fgets(input, MAX_INPUT_LEN, stdin))
            break;
        input[strcspn(input, "\n")] = 0;

        // clock_t start = clock();
        time_t start = time(NULL);
        // printf("input: %s\n", input);
        int status = execute_command(input, R, C, &sheet);

        if (status == 1)
            break;

        // clock_t end = clock();
        // double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

        time_t end = time(NULL);
        double time_taken = difftime(end, start);
        //  && strcmp(input, "disable_output") != 0 && strcmp(input, "enable_output") != 0
        if (flag == 1)
        {
            print_sheet(&sheet);
        }

        if (status == 0 || status == -2)
        {

            printf("[%0.2f] (ok) > ", time_taken);
        }
        else if (status == -4)
        {
            printf("[%0.2f] (Loop Detected!) > ", time_taken);
        }
        else
        {
            printf("[%0.2f] (Invalid Input) > ", time_taken);
        }
    }
    return 0;
}
#endif