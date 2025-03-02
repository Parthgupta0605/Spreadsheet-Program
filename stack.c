#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cell.h"

// Stack node structure
struct stack_node {
    struct cell *cell;
    struct stack_node *next;
};

// Push to stack of dependents of a cell
void push_dependent(struct cell *c, struct cell *dep) {
    struct stack_node *new_node = (struct stack_node *)malloc(sizeof(struct stack_node));
    new_node->cell = dep;
    new_node->next = c->dependents;
    c->dependents = new_node;
}

// Pop from stack of dependents of a cell
struct cell* pop_dependent(struct cell *cell) {
    if (cell->dependents == NULL) return NULL;
    struct stack_node *temp = cell->dependents;
    cell->dependents = cell->dependents->next;
    struct cell *dep_cell = temp->cell;
    free(temp);
    return dep_cell;
} //returns pointer to cell of popped dependent

// Push to stack
void push(struct stack_node **stack, struct cell *cell) {
    struct stack_node *new_node = (struct stack_node *)malloc(sizeof(struct stack_node));
    new_node->cell = cell;
    new_node->next = *stack;
    *stack = new_node;
}

// Pop from stack
struct cell *pop(struct stack_node **stack) {
    if (*stack == NULL) {
        return NULL;
    }
    struct stack_node *top = *stack;
    struct cell *cell = top->cell;
    *stack = top->next;
    free(top);
    return cell;
}//returns pointer to cell of popped node

//CLEARS DEPENDENTS LIST OF A CELL
void free_dependents(struct cell *c) {
    while (c->dependents != NULL) {
        pop_dependent(c);
    }
}