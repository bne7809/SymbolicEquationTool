#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <stack>

enum symbol_type {
    OP, VAR, NUM, FUNC, SPACE
};

enum precedence_type {
    PREC_EQUAL, PREC_LESS, PREC_GREATER, PREC_INVALID
};

struct token {
    symbol_type type;
    std::string expr;
};

std::vector<token> gen_tokens(const std::string& expr);
std::vector<token> postfix_convert(const std::vector<token>& tokens);

symbol_type get_type(char c);

bool is_binary_op(char c);
bool is_left_associative(char op);
bool is_commutative(char op);

precedence_type right_op_prec(char op1, char op2);