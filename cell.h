#ifndef CELL_H
#define CELL_H

#define MAX_INPUT_LEN 35
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Cell structure
struct cell {
    int val; // Value of the cell
    char expression[MAX_INPUT_LEN]; // Expression of the cell
    int status; // Status of the cell to determine if it has ERR or not
    struct avl_node *dependencies; // AVL tree of dependencies
    struct stack_node *dependents; // Stack of dependents
};

#endif