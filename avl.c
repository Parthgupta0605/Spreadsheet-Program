//Including the necessary libraries and header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cell.h"

//Defining the structure of the AVL node
struct avl_node {
    struct cell *cell;
    struct avl_node *left;
    struct avl_node *right;
    int height;
};

//Calculating the row and column of the cell
int calculate_row(struct cell *cell, struct cell ***sheet, int C) {
    return (cell - &(*sheet)[0][0]) / C;
}

int calculate_col(struct cell *cell, struct cell ***sheet, int C) {
    return (cell - &(*sheet)[0][0]) % C;
}

//Defining the functions to be used in the AVL tree

//Function to calculate the height of the node
int height(struct avl_node *Node) {
    if (Node == NULL)
        return 0;
    else 
        return (*Node).height;
}

//Function to calculate the maximum of two integers
int max(int a, int b) {
    if (a > b)
        return a;
    else 
        return b;
}

//Function to create a new node
struct avl_node* new_node(struct cell *cell) {
    struct avl_node* node = (struct avl_node*)malloc(sizeof(struct avl_node));
    node->cell = cell;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}

//Function to perform right rotation
struct avl_node *rotate_right(struct avl_node *node) {
    struct avl_node *x = (*node).left;
    struct avl_node *T2 = x->right;
    x->right = node;
    (*node).left = T2;
    (*node).height = max(height((*node).left), height((*node).right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    return x;
}

//Function to perform left rotation
struct avl_node *rotate_left(struct avl_node *x) {
    struct avl_node *y = x->right;
    struct avl_node *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    return y;
}

//Function to get balance of AVL
int get_balance(struct avl_node *Node) {
    if (Node == NULL)
        return 0;
    else 
        return height(Node->left) - height(Node->right);
}

//Function to insert a new node in the AVL tree and balance it
struct avl_node* insert(struct avl_node* node, struct cell *cell, struct cell ***sheet, int C) {
    if (node == NULL){
        return new_node(cell);
    }

    int cell_row = calculate_row(cell, sheet, C);
    int cell_col = calculate_col(cell, sheet, C);
    int node_row = calculate_row(node->cell, sheet, C);
    int node_col = calculate_col(node->cell, sheet,C);
    
    if (cell_row < node_row || (cell_row == node_row && cell_col < node_col))
        node->left = insert(node->left, cell, sheet, C);
    else if (cell_row > node_row || (cell_row == node_row && cell_col > node_col))
        node->right = insert(node->right, cell, sheet, C);
    else 
        return node;

    node->height = 1 + max(height(node->left), height(node->right));

    int node_left_row = calculate_row(node->left->cell, sheet, C);
    int node_right_row = calculate_row(node->right->cell, sheet, C);
    int node_left_col = calculate_col(node->left->cell, sheet, C);
    int node_right_col = calculate_col(node->right->cell, sheet, C);

    int balance = get_balance(node);

    if (balance > 1 && (cell_row < node_left_row || (cell_row == node_left_row && cell_col < node_left_col)))
        return rotate_right(node);
    if (balance < -1 && (cell_row > node_right_row || (cell_row == node_right_row && cell_col > node_right_col)))
        return rotate_left(node);
    if (balance > 1 && (cell_row > node_left_row || (cell_row == node_left_row && cell_col > node_left_col))) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (balance < -1 && (cell_row < node_right_row || (cell_row == node_right_row && cell_col < node_right_col))) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }
    return node;
}

//Function to find a node in the AVL tree
struct avl_node* find(struct avl_node* root, int row, int col, struct cell ***sheet, int C) {
    if (root == NULL)
        return NULL;
    int root_row = calculate_row(root->cell, sheet, C);
    int root_col = calculate_col(root->cell, sheet, C);
    if (root_row == row && root_col == col)
        return root;
    if (row < root_row || (row == root_row && col < root_col))
        return find(root->left, row, col, sheet, C);
    return find(root->right, row, col, sheet, C);
}

//Function to find the minimum value node in the AVL tree
struct avl_node* min_value_node(struct avl_node* node) {
    struct avl_node* current = node;
    while (current->left != NULL)
        current = current->left;
    return current;
}

//Function to delete a node from the AVL tree and balance it
struct avl_node* delete_node(struct avl_node* root, int row, int col, struct cell ***sheet, int C) {
    if (root == NULL)
        return root;
    int root_row = calculate_row(root->cell, sheet, C);
    int root_col = calculate_col(root->cell, sheet, C);
    if (row < root_row || (row == root_row && col < root_col))
        root->left = delete_node(root->left, row, col, sheet, C);
    else if (row > root_row || (row == root_row && col > root_col))
        root->right = delete_node(root->right, row, col, sheet, C);
    else {
        if ((root->left == NULL) || (root->right == NULL)) {
            struct avl_node *temp;
            if (root->left) {
                temp = root->left;
            } else {
                temp = root->right;
            }

            if (temp == NULL) {
                temp = root;
                root = NULL;
            } else
                *root = *temp;
            free(temp);
        } else {
            struct avl_node* temp = min_value_node(root->right);
            root->cell = temp->cell;
            root->right = delete_node(root->right, calculate_row(temp->cell, sheet, C), calculate_col(temp->cell, sheet, C), sheet, C);
        }
    }
    if (root == NULL)
        return root;
    root->height = 1 + max(height(root->left), height(root->right));
    int balance = get_balance(root);
    if (balance > 1 && get_balance(root->left) >= 0)
        return rotate_right(root);
    if (balance > 1 && get_balance(root->left) < 0) {
        root->left = rotate_left(root->left);
        return rotate_right(root);
    }
    if (balance < -1 && get_balance(root->right) <= 0)
        return rotate_left(root);
    if (balance < -1 && get_balance(root->right) > 0) {
        root->right = rotate_right(root->right);
        return rotate_left(root);
    }
    return root;
}
