#include "print_util.h"

void print_tokens(const std::vector<token>& tokens) {
    for (int i = 0; i < tokens.size(); i++) {
        std::cout << "Expression: " << tokens[i].expr << ", Type: " << tokens[i].type << '\n';
    }
}

void print_expr_tree(expr_node* root) {
    struct expr_node_print {
        expr_node* node;
        int off;
    };

    int num_levels = num_expr_tree_levels(root);
    int offset = num_levels * 5;
    int curr_level_num = 1;
    int next_level_num = 0;
    int branch_len = num_levels;

    std::queue<expr_node_print> traversal_queue;
    std::deque<int> branch_inf;
    std::stringstream ss;
    traversal_queue.push({ root, offset });

    auto insert_space = [&](int n) {
        for (int j = 0; j < n; j++) {
            ss << ' ';
        }
    };

    while (!traversal_queue.empty()) {
        expr_node_print prev;
        for (int i = 0; i < curr_level_num; i++) {
            expr_node_print curr = traversal_queue.front();
            if (i == 0) {
                insert_space(curr.off);
            } else {
                insert_space(curr.off - (prev.off + (prev.node->t.expr.size() >> 1)) - 1);
            }

            ss << curr.node->t.expr;

            if (curr.node->left != nullptr) {
                expr_node_print left, right;
                left.node = curr.node->left;
                left.off = curr.off - (branch_len + 1) - (curr.node->left->t.expr.size() >> 1);
                right.node = curr.node->right;
                right.off = curr.off + (branch_len + 1) - (curr.node->right->t.expr.size() >> 1);

                traversal_queue.push(left);
                traversal_queue.push(right);
                branch_inf.push_back(curr.off - 1);
                branch_inf.push_back(curr.off + 1);
                next_level_num += 2;
            }

            prev = curr;
            traversal_queue.pop();
        }

        std::cout << ss.str() << '\n';
        ss.str("");
        curr_level_num = next_level_num;
        next_level_num = 0;

        int prev_off;
        for (int j = 0; j < branch_len; j++) {
            for (int i = 0; i < branch_inf.size(); i++) {
                if (i == 0) {
                    insert_space(branch_inf[i]);
                } else {
                    insert_space(branch_inf[i] - prev_off - 1);
                }

                if ((i % 2) == 0) {
                    ss << '/';
                } else {
                    ss << '\\';
                }

                prev_off = branch_inf[i];
            }

            std::cout << ss.str() << '\n';
            ss.str("");

            for (int i = 0; i < branch_inf.size(); i++) {
                if ((i % 2) == 0) {
                    branch_inf[i]--;
                } else {
                    branch_inf[i]++;
                }
            }
        }
        branch_inf.clear();
        branch_len--;
    }
}