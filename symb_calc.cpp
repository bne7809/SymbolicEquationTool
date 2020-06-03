#include "symb_calc.h"

float bin_symb_op(float left, float right, char op) {
    if (op == '+') {
        return left + right;
    } else if (op == '-') {
        return left - right;
    } else if (op == '*') {
        return left * right;
    } else if (op == '/') {
        return left / right;
    } else if (op == '^') {
        return std::powf(left, right);
    } else {
        std::cerr << "Invalid operation.\n";

        return -1.0f;
    }
}

float symb_func(float arg, const std::string& func) {
    int sign = 1;
    std::string cmp_str = "";
    if (func[0] == '-') {
        sign = -1;
        cmp_str += '-';
    }
    if (func == cmp_str + "sin") {
        return sign * std::sinf(arg);
    } else if (func == cmp_str + "cos") {
        return sign * std::cosf(arg);
    } else if (func == cmp_str + "exp") {
        return sign * std::expf(arg);
    } else if (func == cmp_str + "log") {
        return sign * std::logf(arg);
    } else if (func == "neg") {
        return -1 * arg;
    } else {
        std::cerr << "Invalid function.\n";

        return -1.0f;
    }
}

float calc_expr(const std::string& expr) {
    std::vector<token> tokens = postfix_convert(gen_tokens(expr));
    
    size_t tokens_len = tokens.size();
    std::stack<float> num_stack;

    for (int i = 0; i < tokens_len; i++) {
        token t = tokens[i];

        if (t.type == NUM) {
            num_stack.push(std::stof(t.expr.c_str()));
        } else if (t.type == OP) {
            float operand_right = num_stack.top();
            num_stack.pop();
            float operand_left = num_stack.top();
            num_stack.pop();

            num_stack.push(bin_symb_op(operand_left, operand_right, t.expr[0]));
        } else if (t.type == FUNC) {
            float arg = num_stack.top();
            num_stack.pop();

            num_stack.push(symb_func(arg, t.expr));
        }
    }

    return num_stack.top();
}