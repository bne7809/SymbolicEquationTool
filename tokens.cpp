#include "tokens.h"

symbol_type get_type(char c) {
    if ((c >= '0' && c <= '9') || c == '.') {
        return NUM;
    } else if (c == '*' || c == '/' || c == '+' || c == '-' || c == '(' || c == ')' || c == '^') {
        return OP;
    } else if (c == ' ') {
        return SPACE;
    } else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
        return VAR;
    }
}

bool is_binary_op(char c) {
    if (c == '*' || c == '/' || c == '+' || c == '-' || c == '^') return true;
    else return false;
}

precedence_type right_op_prec(char op1, char op2) {
    if (op1 == '+' || op1 == '-') {
        if (op2 == '+' || op2 == '-') {
            return PREC_EQUAL;
        } else {
            return PREC_GREATER;
        }
    } else if (op1 == '/' || op1 == '*') {
        if (op2 == '/' || op2 == '*') {
            return PREC_EQUAL;
        } else if (op2 == '^') {
            return PREC_GREATER;
        } else {
            return PREC_LESS;
        }
    } else if (op1 == '^') {
        if (op2 == '^') {
            return PREC_EQUAL;
        } else {
            return PREC_LESS;
        }
    } else {
        std::cerr << "Invalid comparison: " << op1 << " with " << op2 << '\n';

        return PREC_INVALID;
    }
}

bool is_left_associative(char op) {
    if (op == '-' || op == '+' || op == '/' || op == '*') return true;
    else return false;
}

bool is_commutative(char op) {
    if (op == '+' || op == '*') return true;
    else return false;
}

std::vector<token> gen_tokens(const std::string& expr) {
    std::vector<token> ret;
    std::stringstream ss;
    symbol_type ct = get_type(expr[0]);
    char prev_c = expr[0];

    for (int i = 0; i < expr.size(); i++) {
        char c = expr[i];
        symbol_type st = get_type(c);

        if (st == NUM) {
            ss << c;
        } else if (st == OP) {
            if (ct == NUM) {
                token t1;
                t1.expr = ss.str();
                t1.type = NUM;
                ret.push_back(t1);
                ss.str("");

                token t2;
                if (c == '-') {
                    t2.expr = '+';
                    ss << c;
                } else {
                    t2.expr = c;
                }
                t2.type = OP;
                ret.push_back(t2);
            } else if (ct == VAR) {
                token t2;
                if (c == '-') {
                    t2.expr = '+';
                    ss << c;
                } else {
                    t2.expr = c;
                }
                t2.type = OP;
                ret.push_back(t2);
            } else if (ct == OP || i == 0) {
                if (prev_c == ')') {
                    token t7;
                    if (c == '-') {
                        t7.expr = '+';
                        ss << c;
                    } else {
                        t7.expr = c;
                    }
                    t7.type = OP;
                    ret.push_back(t7);
                } else {
                    if (c != '(') {
                        ss << c;
                    } else {
                        token t8;
                        if (ss.str()[0] == '-') {
                            token t9;
                            t9.expr = "neg";
                            t9.type = FUNC;
                            ret.push_back(t9);
                            ss.str("");
                        }
                        t8.expr = c;
                        t8.type = OP;
                        ret.push_back(t8);
                    }
                }
            } else {
                token t2;
                t2.expr = c;
                t2.type = OP;
                ret.push_back(t2);
            }
        } else if (st == VAR) {
            bool is_func = false;
            if (c == 's' && (i < expr.size() - 2)) {
                if (expr[i + 1] == 'i' && expr[i + 2] == 'n') {
                    i += 2;
                    token t5;
                    t5.expr = "";
                    if (ss.str()[0] == '-') t5.expr += '-';
                    t5.expr += "sin";
                    t5.type = FUNC;
                    ret.push_back(t5);
                    ss.str("");
                    is_func = true;
                }
            } else if (c == 'c' && (i < expr.size() - 2)) {
                if (expr[i + 1] == 'o' && expr[i + 2] == 's') {
                    i += 2;
                    token t5;
                    t5.expr = "";
                    if (ss.str()[0] == '-') t5.expr += '-';
                    t5.expr += "cos";
                    t5.type = FUNC;
                    ret.push_back(t5);
                    ss.str("");
                    is_func = true;
                }
            } else if (c == 'e' && (i < expr.size() - 2)) {
                if (expr[i + 1] == 'x' && expr[i + 2] == 'p') {
                    i += 2;
                    token t5;
                    t5.expr = "";
                    if (ss.str()[0] == '-') t5.expr += '-';
                    t5.expr += "exp";
                    t5.type = FUNC;
                    ret.push_back(t5);
                    ss.str("");
                    is_func = true;
                }
            } else if (c == 'l' && (i < expr.size() - 2)) {
                if (expr[i + 1] == 'o' && expr[i + 2] == 'g') {
                    i += 2;
                    token t5;
                    t5.expr = "";
                    if (ss.str()[0] == '-') t5.expr += '-';
                    t5.expr += "log";
                    t5.type = FUNC;
                    ret.push_back(t5);
                    ss.str("");
                    is_func = true;
                }
            }

            if (!is_func) {
                token t4;
                t4.expr = "";
                if (ss.str()[0] == '-') t4.expr += '-';
                t4.expr += c;
                ss.str("");
                t4.type = VAR;
                ret.push_back(t4);
            }
        }

        if ((i == expr.size() - 1) && (st == NUM)) {
            token t3;
            t3.expr = ss.str();
            t3.type = st;
            ret.push_back(t3);
            ss.str("");
        }

        if (st != SPACE) {
            ct = st;
            prev_c = c;
        }
    }

    return ret;
}

std::vector<token> postfix_convert(const std::vector<token>& tokens) {
    std::vector<token> ret;
    std::queue<token> output;
    std::stack<token> ops;

    for (int i = 0; i < tokens.size(); i++) {
        token t = tokens[i];

        if (t.type == NUM || t.type == VAR) {
            output.push(t);
        } else if (t.type == FUNC) {
            ops.push(t);
        } else if (t.type == OP) {
            bool is_bin_op = is_binary_op(t.expr[0]);

            if (is_bin_op) {
                if (!ops.empty()) {
                    while (((right_op_prec(t.expr[0], ops.top().expr[0]) == PREC_GREATER) ||
                        ((right_op_prec(t.expr[0], ops.top().expr[0]) == PREC_EQUAL) && is_left_associative(t.expr[0])))
                        && ops.top().expr[0] != '(')
                    {
                        output.push(ops.top());
                        ops.pop();

                        if (ops.empty()) break;
                    }
                }
                ops.push(t);
            } else {
                if (t.expr[0] == '(') {
                    ops.push(t);
                } else if (t.expr[0] == ')') {
                    while (ops.top().expr[0] != '(') {
                        output.push(ops.top());
                        ops.pop();
                    }

                    if (ops.top().expr[0] == '(') {
                        ops.pop();

                        if (!ops.empty()) {
                            if (ops.top().type == FUNC) {
                                output.push(ops.top());
                                ops.pop();
                            }
                        }
                    }
                }
            }
        }
    }

    while (!ops.empty()) {
        output.push(ops.top());
        ops.pop();
    }

    while (!output.empty()) {
        ret.push_back(output.front());
        output.pop();
    }

    return ret;
}