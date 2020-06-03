#include "symb_calc.h"

int main() {
    expr_node* root = gen_expr_tree("x^x");
    print_expr_tree(root);
    derivative(root, 'x');
    resolve_numbers(root);
    print_expr_tree(root);
    resolve_exp_num_den(root);

    std::cout << expr_tree_to_infix(root) << '\n';

    return 0;
}