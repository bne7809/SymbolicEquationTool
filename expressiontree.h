#pragma once

#include "tokens.h"
#include <map>

struct expr_node {
    token t;
    expr_node* left;
    expr_node* right;
    expr_node* parent;
};

struct md_expr_node {
    expr_node* node;
    int level;
};

enum branch_dir {
    LEFT, RIGHT, SINGLE_PARAM, TREE_BEGIN
};

int num_expr_tree_levels(expr_node* root);

expr_node* gen_expr_tree(const std::string& expr);
expr_node* gen_rep_expr_tree(std::queue<expr_node*>& eq, char op);
expr_node* copy_node(expr_node* node);

void expr_tree_distribute(expr_node* root);
void expr_tree_apply_neg(expr_node*& root);
void resolve_numbers(expr_node*& root);
void resolve_exp_num_den(expr_node*& root);
void derivative(expr_node*& root, char wrt);

std::string expr_tree_to_infix(expr_node* curr);

bool is_constant(expr_node* node, char wrt);