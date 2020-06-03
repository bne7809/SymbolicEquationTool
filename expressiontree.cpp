#include "expressiontree.h"
#include "symb_calc.h"

int num_expr_tree_levels(expr_node* root) {
    if (root->t.type == NUM || root->t.type == VAR) {
        return 1;
    } else if (root->t.type == OP) {
        int left = 1 + num_expr_tree_levels(root->left);
        int right = 1 + num_expr_tree_levels(root->right);

        return ((left > right) ? left : right);
    } else {
        return 1 + num_expr_tree_levels(root->left);
    }
}

expr_node* gen_expr_tree(const std::string& expr) {
    std::vector<token> tokens = postfix_convert(gen_tokens(expr));
    size_t tokens_len = tokens.size();
    expr_node* nodes = new expr_node[tokens_len];
    std::stack<expr_node*> node_stack;
    int j = 0;

    for (int i = 0; i < tokens_len; i++) {
        token t = tokens[i];

        if (t.type == NUM || t.type == VAR) {
            nodes[j].t = t;
            nodes[j].left = nullptr;
            nodes[j].right = nullptr;
            node_stack.push(&nodes[j]);
            j++;
        } else if (t.type == OP) {
            expr_node* operand_right = node_stack.top();
            node_stack.pop();
            expr_node* operand_left = node_stack.top();
            node_stack.pop();

            nodes[j].t = t;
            nodes[j].left = operand_left;
            nodes[j].right = operand_right;
            operand_right->parent = &nodes[j];
            operand_left->parent = &nodes[j];

            node_stack.push(&nodes[j]);
            j++;
        } else if (t.type == FUNC) {
            expr_node* arg = node_stack.top();
            node_stack.pop();

            nodes[j].t = t;
            nodes[j].left = arg;
            nodes[j].right = nodes[j].left;
            arg->parent = &nodes[j];

            node_stack.push(&nodes[j]);
            j++;
        }
    }

    node_stack.top()->parent = nullptr;
    return node_stack.top();
}

void expr_tree_distribute(expr_node* root) {
    if (root->t.type == OP) {
        if (root->t.expr[0] == '^' || root->t.expr[0] == '+' || root->t.expr[0] == '-') {
            expr_tree_distribute(root->left);
            expr_tree_distribute(root->right);
        } else {
            if (root->left->t.type == OP || root->right->t.type == OP) {
                brnch1:
                expr_node* out_ptr;
                expr_node* in_ptr;
                char div_or_mul = root->t.expr[0];

                if (root->left->t.expr[0] == '+') {
                    in_ptr = root->left;
                    out_ptr = root->right;
                } else if (root->right->t.expr[0] == '+') {
                    if (div_or_mul == '*') {
                        in_ptr = root->right;
                        out_ptr = root->left;
                    } else {
                        expr_tree_distribute(root->left);
                        expr_tree_distribute(root->right);
                        if (root->left->t.expr[0] == '+') goto brnch1;
                        return;
                    }
                } else {
                    expr_tree_distribute(root->left);
                    expr_tree_distribute(root->right);
                    if (root->left->t.expr[0] == '+' || root->right->t.expr[0] == '+') goto brnch1;
                    return;
                }

                expr_node* in_left = in_ptr->left;
                expr_node* in_right = in_ptr->right;

                root->t.expr = in_ptr->t.expr;
                expr_node* dist_left = new expr_node;
                expr_node* dist_right = new expr_node;
                expr_node* out_ptr2 = new expr_node;
                out_ptr2->t = out_ptr->t;
                out_ptr2->left = out_ptr->left;
                out_ptr2->right = out_ptr->right;

                dist_left->t.expr = div_or_mul;
                dist_left->t.type = OP;
                dist_left->right = out_ptr;
                out_ptr->parent = dist_left;
                dist_left->left = in_left;
                in_left->parent = dist_left;
                dist_left->parent = root;

                dist_right->t.expr = div_or_mul;
                dist_right->t.type = OP;
                dist_right->right = out_ptr2;
                out_ptr2->parent = dist_right;
                dist_right->left = in_right;
                in_right->parent = dist_right;
                dist_right->parent = root;
                root->left = dist_left;
                root->right = dist_right;

                if (root->parent != nullptr) {
                    if (root->parent->t.type == OP) {
                        if (root->parent->t.expr[0] == '*' || root->parent->t.expr[0] == '/') {
                            expr_tree_distribute(root->parent);
                        }
                    }
                }

                expr_tree_distribute(root->left);
                expr_tree_distribute(root->right);
            } else {
                expr_tree_distribute(root->left);
                expr_tree_distribute(root->right);
            }
        }
    } else if (root->t.type == FUNC) {
        expr_tree_distribute(root->left);
    } else {
        return;
    }
}

std::string expr_tree_to_infix(expr_node* curr) {
    if (curr->t.type == NUM || curr->t.type == VAR) {
        return curr->t.expr;
    } else if (curr->t.type == OP) {
        if (curr->parent == nullptr) {
            if (curr->t.expr[0] != '/') {
                return expr_tree_to_infix(curr->left) + curr->t.expr + expr_tree_to_infix(curr->right);
            } else {
                if (curr->right->t.expr[0] == '*') {
                    return expr_tree_to_infix(curr->left) + curr->t.expr + '(' + expr_tree_to_infix(curr->right) + ')';
                } else {
                    return expr_tree_to_infix(curr->left) + curr->t.expr + expr_tree_to_infix(curr->right);
                }
            }
        } else if (curr->parent->t.type == FUNC) {
            if (curr->t.expr[0] != '/') {
                return expr_tree_to_infix(curr->left) + curr->t.expr + expr_tree_to_infix(curr->right);
            } else {
                if (curr->right->t.expr[0] == '*') {
                    return expr_tree_to_infix(curr->left) + curr->t.expr + '(' + expr_tree_to_infix(curr->right) + ')';
                } else {
                    return expr_tree_to_infix(curr->left) + curr->t.expr + expr_tree_to_infix(curr->right);
                }
            }
        } else {
            if (right_op_prec(curr->t.expr[0], curr->parent->t.expr[0]) == PREC_GREATER) {
                return '(' + expr_tree_to_infix(curr->left) + curr->t.expr + expr_tree_to_infix(curr->right) + ')';
            } else {
                if (curr->t.expr[0] != '/') {
                    return expr_tree_to_infix(curr->left) + curr->t.expr + expr_tree_to_infix(curr->right);
                } else {
                    if (curr->right->t.expr[0] == '*') {
                        return expr_tree_to_infix(curr->left) + curr->t.expr + '(' + expr_tree_to_infix(curr->right) + ')';
                    } else {
                        return expr_tree_to_infix(curr->left) + curr->t.expr + expr_tree_to_infix(curr->right);
                    }
                }
            }
        }
    } else {
        if (curr->t.expr == "neg") {
            return "-(" + expr_tree_to_infix(curr->left) + ')';
        } else {
            return curr->t.expr + '(' + expr_tree_to_infix(curr->left) + ')';
        }
    }
}

bool is_constant(expr_node* node, char wrt) {
    if (node->t.type == NUM) {
        return true;
    } else if (node->t.type == VAR) {
        int i = 0;
        if (node->t.expr[0] == '-') i = 1;
        return (node->t.expr[i] != wrt);
    } else if (node->t.type == OP) {
        if (is_constant(node->left, wrt)) {
            return is_constant(node->right, wrt);
        } else {
            return false;
        }
    } else {
        return is_constant(node->left, wrt);
    }
}

void expr_tree_apply_neg_impl(expr_node*& root, bool apply, branch_dir dir) {
    if (root->t.type == NUM || root->t.type == VAR) {
        if (apply) {
            if (root->t.expr[0] == '-') {
                root->t.expr = root->t.expr.substr(1, root->t.expr.size() - 1);
            } else {
                root->t.expr = '-' + root->t.expr;
            }
        }
    } else if (root->t.type == OP) {
        if (apply) {
            if (root->t.expr[0] == '*' || root->t.expr[0] == '/') {
                expr_tree_apply_neg_impl(root->left, true, LEFT);
                expr_tree_apply_neg_impl(root->right, false, RIGHT);
            } else if (root->t.expr[0] == '^') {
                expr_node* ul = copy_node(root->left);
                expr_node* ur = copy_node(root->right);

                expr_node* pown = new expr_node;
                pown->t.expr = '^';
                pown->t.type = OP;
                pown->left = ul;
                ul->parent = pown;
                pown->right = ur;
                ur->parent = pown;
                
                expr_node* mon = new expr_node;
                mon->t.expr = "-1";
                mon->t.type = NUM;
                mon->left = nullptr;
                mon->right = nullptr;

                root->t.expr = '*';
                root->left = mon;
                mon->parent = root;
                root->right = pown;
                pown->parent = root;
                
                expr_tree_apply_neg_impl(root->right->left, false, LEFT);
                expr_tree_apply_neg_impl(root->right->right, false, RIGHT);
            } else {
                expr_tree_apply_neg_impl(root->left, true, LEFT);
                expr_tree_apply_neg_impl(root->right, true, RIGHT);
            }
        } else {
            expr_tree_apply_neg_impl(root->left, false, LEFT);
            expr_tree_apply_neg_impl(root->right, false, RIGHT);
        }
    } else {
        if (apply) {
            if (root->t.expr == "neg") {
                if (dir == LEFT) {
                    root->parent->left = root->left;
                } else if (dir == RIGHT) {
                    root->parent->right = root->left;
                } else if (dir == SINGLE_PARAM) {
                    root->parent->left = root->left;
                    //root->parent->right = root->parent->left;
                }
                if(dir != TREE_BEGIN) root->parent = root->parent->parent;
                //root->left->parent = root->parent;
                //root->right->parent = root->left->parent;
                if (dir == TREE_BEGIN) { 
                    root->left->parent = root->parent;
                    root = root->left; 
                }
                //root->right = root;

                expr_tree_apply_neg_impl(root, false, dir);
            } else {
                if (root->t.expr[0] == '-') {
                    root->t.expr = root->t.expr.substr(1, root->t.expr.size() - 1);
                } else {
                    root->t.expr = '-' + root->t.expr;
                }

                expr_tree_apply_neg_impl(root->left, false, SINGLE_PARAM);
            }
        } else {
            if (root->t.expr == "neg") {
                if (dir == LEFT) {
                    root->parent->left = root->left;
                } else if (dir == RIGHT) {
                    root->parent->right = root->left;
                } else if (dir == SINGLE_PARAM) {
                    root->parent->left = root->left;
                    //root->parent->right = root->parent->left;
                }
                if(dir != TREE_BEGIN) root->parent = root->parent->parent;
                //root->left->parent = root->parent;
                //root->right->parent = root->left->parent;
                if (dir == TREE_BEGIN) {
                    root->left->parent = root->parent;
                    root = root->left; 
                }
                //root->right = root;

                expr_tree_apply_neg_impl(root, true, dir);
            } else {
                expr_tree_apply_neg_impl(root->left, false, SINGLE_PARAM);
            }
        }
    }
}

void expr_tree_apply_neg(expr_node*& root) {
    expr_tree_apply_neg_impl(root, false, TREE_BEGIN);
}

expr_node* gen_rep_expr_tree(std::queue<expr_node*>& eq, char op) {
    if (eq.empty()) {
        return nullptr;
    } else if (eq.size() == 1) {
        expr_node* ret = eq.front();
        eq.pop();

        return ret;
    } else {
        size_t eq_len = eq.size();
        expr_node* op_nodes = new expr_node[eq_len - 1];
        expr_node* first = eq.front();
        eq.pop();
        expr_node* second = eq.front();
        eq.pop();
        op_nodes[0].left = first;
        op_nodes[0].right = second;
        op_nodes[0].t.expr = op;
        op_nodes[0].t.type = OP;
        for (int i = 1; i < eq_len - 1; i++) {
            op_nodes[i].left = &op_nodes[i - 1];
            op_nodes[i].right = eq.front();
            eq.front()->parent = &op_nodes[i];
            op_nodes[i].t.expr = op;
            op_nodes[i].t.type = OP;
            op_nodes[i - 1].parent = &op_nodes[i];
            eq.pop();
        }

        return &op_nodes[eq_len - 2];
    }
}

expr_node* gen_rep_expr_tree_md(std::queue<md_expr_node>& eq, bool& num_is_one) {
    if (eq.empty()) {
        return nullptr;
    } else {
        std::queue<expr_node*> num, den;
        while (!eq.empty()) {
            if ((eq.front().level % 2) == 0) {
                num.push(eq.front().node);
            } else {
                den.push(eq.front().node);
            }
            eq.pop();
        }

        expr_node* num_node = gen_rep_expr_tree(num, '*');
        expr_node* den_node = gen_rep_expr_tree(den, '*');

        if (num_node != nullptr && den_node == nullptr) {
            num_is_one = false;
            return num_node;
        } else if (num_node == nullptr && den_node != nullptr) {
            expr_node* ret = new expr_node;
            ret->right = den_node;
            den_node->parent = ret;
            ret->left = nullptr;
            ret->t.expr = '/';
            ret->t.type = OP;
            num_is_one = true;

            return ret;
        } else {
            expr_node* ret = new expr_node;
            ret->right = den_node;
            den_node->parent = ret;
            ret->left = num_node;
            num_node->parent = ret;
            ret->t.expr = '/';
            ret->t.type = OP;
            num_is_one = false;

            return ret;
        }
    }
}

void resolve_numbers(expr_node*& root) {
    if (root->t.type == OP) {
        if (root->t.expr[0] == '+') {
            std::queue<expr_node*> tq;
            std::vector<expr_node*> addends;
            tq.push(root->left);
            tq.push(root->right);

            while (!tq.empty()) {
                expr_node* curr = tq.front();

                if (curr->t.expr[0] == '+') {
                    tq.push(curr->left);
                    tq.push(curr->right);
                } else {
                    resolve_numbers(curr);
                    addends.push_back(curr);
                }
                tq.pop();
            }
            
            float res = 0;
            bool res_change = false;
            for (int i = 0; i < addends.size(); i++) {
                if (addends[i]->t.type == NUM) {
                    res += std::stof(addends[i]->t.expr);
                    res_change = true;
                } else {
                    tq.push(addends[i]);
                }
            }

            expr_node* num_node = nullptr;
            expr_node* rest = nullptr;

            if (res_change) {
                num_node = new expr_node;
                num_node->t.expr = std::to_string(res);
                num_node->t.type = NUM;
                num_node->left = nullptr;
                num_node->right = nullptr;
            }

            if (!tq.empty()) {
                rest = gen_rep_expr_tree(tq, '+');
            }

            if (rest != nullptr && (num_node == nullptr || res == 0.0f)) {
                rest->parent = root->parent;
                if (root->parent != nullptr) {
                    if (root->parent->left == root) {
                        root->parent->left = rest;
                    } else {
                        root->parent->right = rest;
                    }
                }
                root = rest;
            } else if (num_node != nullptr && rest == nullptr) {
                num_node->parent = root->parent;
                if (root->parent != nullptr) {
                    if (root->parent->left == root) {
                        root->parent->left = num_node;
                    } else {
                        root->parent->right = num_node;
                    }
                }
                root = num_node;
            } else {
                expr_node* new_root = new expr_node;
                new_root->t.expr = '+';
                new_root->t.type = OP;
                new_root->left = rest;
                rest->parent = new_root;
                new_root->right = num_node;
                num_node->parent = new_root;
                new_root->parent = root->parent;
                if (root->parent != nullptr) {
                    if (root->parent->left == root) {
                        root->parent->left = new_root;
                    } else {
                        root->parent->right = new_root;
                    }
                }
                root = new_root;
            }
        } else if (root->t.expr[0] == '*' || root->t.expr[0] == '/') {
            std::queue<md_expr_node> tq;
            std::vector<md_expr_node> elems;
            tq.push({ root->left, 0 });
            if (root->t.expr[0] == '*') {
                tq.push({ root->right, 0 });
            } else {
                tq.push({ root->right, 1 });
            }
            while (!tq.empty()) {
                if (tq.front().node->t.expr[0] == '*') {
                    tq.push({ tq.front().node->left, tq.front().level });
                    tq.push({ tq.front().node->right, tq.front().level });
                } else if (tq.front().node->t.expr[0] == '/') {
                    tq.push({ tq.front().node->left, tq.front().level });
                    tq.push({ tq.front().node->right, tq.front().level + 1});
                } else {
                    resolve_numbers(tq.front().node);
                    elems.push_back(tq.front());
                }
                tq.pop();
            }
            float res = 1.0f;
            bool res_change = false;
            for (int i = 0; i < elems.size(); i++) {
                if (elems[i].node->t.type == NUM) {
                    if ((elems[i].level % 2) == 0) {
                        res *= std::stof(elems[i].node->t.expr);
                    } else {
                        res /= std::stof(elems[i].node->t.expr);
                    }
                    res_change = true;
                } else {
                    tq.push(elems[i]);
                }
            }

            expr_node* num_node = nullptr;
            expr_node* rest = nullptr;
            bool num_is_one;

            if (res_change) {
                num_node = new expr_node;
                num_node->t.expr = std::to_string(res);
                num_node->t.type = NUM;
                num_node->left = nullptr;
                num_node->right = nullptr;
            }

            if (!tq.empty()) {
                rest = gen_rep_expr_tree_md(tq, num_is_one);
            }
            
            if (res == 0.0f) {
                root->t.expr = '0';
                root->t.type = NUM;
                root->left = nullptr;
                root->right = nullptr;
            } else if (rest != nullptr && (num_node == nullptr || res == 1.0f)) {
                rest->parent = root->parent;
                if (root->parent != nullptr) {
                    if (root->parent->left == root) {
                        root->parent->left = rest;
                    } else {
                        root->parent->right = rest;
                    }
                }
                //resolve_exp_num_den(rest);
                root = rest;
            } else if (rest != nullptr && res == -1.0f) {
                rest->parent = root->parent;
                if (root->parent != nullptr) {
                    if (root->parent->left == root) {
                        root->parent->left = rest;
                        expr_tree_apply_neg_impl(rest, true, LEFT);
                    } else {
                        root->parent->right = rest;
                        expr_tree_apply_neg_impl(rest, true, RIGHT);
                    }
                } else {
                    expr_tree_apply_neg_impl(rest, true, TREE_BEGIN);
                }
                root = rest;
            } else if (num_node != nullptr && rest == nullptr) {
                num_node->parent = root->parent;
                if (root->parent != nullptr) {
                    if (root->parent->left == root) {
                        root->parent->left = num_node;
                    } else {
                        root->parent->right = num_node;
                    }
                }
                root = num_node;
            } else {
                if (num_is_one) {
                    rest->left = num_node;
                    num_node->parent = rest;
                    rest->parent = root->parent;
                    if (root->parent != nullptr) {
                        if (root->parent->left == root) {
                            root->parent->left = rest;
                        } else {
                            root->parent->right = rest;
                        }
                    }
                    root = rest;
                } else {
                    expr_node* new_root = new expr_node;
                    new_root->t.expr = '*';
                    new_root->t.type = OP;
                    new_root->right = rest;
                    rest->parent = new_root;
                    new_root->left = num_node;
                    num_node->parent = new_root;
                    new_root->parent = root->parent;
                    if (root->parent != nullptr) {
                        if (root->parent->left == root) {
                            root->parent->left = new_root;
                        } else {
                            root->parent->right = new_root;
                        }
                    }
                    root = new_root;
                }
            }
            
        } else {
            resolve_numbers(root->left);
            resolve_numbers(root->right);
            if (root->left->t.type == NUM && root->right->t.type == NUM) {
                root->t.expr = std::to_string(std::powf(std::stof(root->left->t.expr), std::stof(root->right->t.expr)));
                root->t.type = NUM;
                root->left = nullptr;
                root->right = nullptr;
            } else if (root->left->t.expr == "0") {
                root->t.expr = '0';
                root->t.type = NUM;
                root->left = nullptr;
                root->right = nullptr;
            } else if (root->left->t.expr == "1.000000" || root->left->t.expr == "1") {
                root->t.expr = '1';
                root->t.type = NUM;
                root->left = nullptr;
                root->right = nullptr;
            } else if (root->right->t.expr == "1.000000" || root->right->t.expr == "1") {
                root->left->parent = root->parent;
                if (root->parent != nullptr) {
                    if (root->parent->left == root) {
                        root->parent->left = root->left;
                    } else {
                        root->parent->right = root->left;
                    }
                }
                root = root->left;
            } else if (root->right->t.expr == "0") {
                root->t.expr = '1';
                root->t.type = NUM;
                root->left = nullptr;
                root->right = nullptr;
            }
        }
    } else if (root->t.type == FUNC) {
        resolve_numbers(root->left);
        root->right = root->left;

        if (root->left->t.type == NUM) {
            root->t.expr = std::to_string(symb_func(std::stof(root->left->t.expr), root->t.expr));
            root->t.type = NUM;
            root->left = nullptr;
            root->right = nullptr;
        }
    } else {
        return;
    }
}

void resolve_exp_num_den(expr_node*& root) {
    if (root->t.type == OP) {
        if (root->t.expr[0] == '+') {
            resolve_exp_num_den(root->left);
            resolve_exp_num_den(root->right);
        } else if (root->t.expr[0] == '*' || root->t.expr[0] == '/') {
            /*if (root->left->t.expr[0] == '+') {
                resolve_numbers(root->left);
                return;
            } else if (root->right->t.expr[0] == '+') {
                resolve_numbers(root->right);
                return;
            }*/
            struct const_inf {
                int nexp;

                const_inf() : nexp(0) {}
            };

            std::map<char, const_inf> cm;
            std::queue<md_expr_node> tq;
            std::vector<char> vars;
            std::queue<expr_node*> eq;
            int tot_sgn = 0;
            tq.push({ root->left, 0 });
            if (root->t.expr[0] == '*') {
                tq.push({ root->right, 0 });
            } else {
                tq.push({ root->right, 1 });
            }

            while (!tq.empty()) {
                md_expr_node curr = tq.front();
                if (curr.node->t.type == OP) {
                    if (curr.node->t.expr[0] == '+') {
                        resolve_exp_num_den(curr.node->left);
                        resolve_exp_num_den(curr.node->right);
                    } else if (curr.node->t.expr[0] == '*' || curr.node->t.expr[0] == '/') {
                        tq.push({ curr.node->left, curr.level });
                        if (curr.node->t.expr[0] == '*') {
                            tq.push({ curr.node->right, curr.level });
                        } else {
                            tq.push({ curr.node->right, curr.level + 1});
                        }
                    } else {
                        brnch2:
                        if (curr.node->right->t.type != NUM) {
                            resolve_exp_num_den(curr.node->right);
                            if (curr.node->right->t.type == NUM) goto brnch2;
                        }
                        if (curr.node->left->t.type != VAR) {
                            resolve_exp_num_den(curr.node->left);
                            if (curr.node->left->t.type == VAR) goto brnch2;
                        }
                        if (curr.node->left->t.type == VAR && curr.node->right->t.type == NUM) {
                            int var_ind = 0;
                            if (curr.node->left->t.expr[0] == '-') var_ind = 1;
                            if (std::find(vars.begin(), vars.end(), curr.node->left->t.expr[var_ind]) == vars.end()) {
                                vars.push_back(curr.node->left->t.expr[var_ind]);
                            }
                            int ie = std::stoi(curr.node->right->t.expr);
                            if ((curr.level % 2) == 0) {
                                cm[curr.node->left->t.expr[var_ind]].nexp += ie;
                            } else {
                                cm[curr.node->left->t.expr[var_ind]].nexp -= ie;
                            }
                            if ((ie % 2) == 1 && (var_ind == 1)) tot_sgn++;
                        }
                    }
                } else if (curr.node->t.type == FUNC) {
                    resolve_exp_num_den(curr.node->left);
                    if (curr.node->t.expr[0] == '-') {
                        curr.node->t.expr = curr.node->t.expr.substr(1, curr.node->t.expr.size() - 1);
                        tot_sgn++;
                    }
                    eq.push(curr.node);
                } else if (curr.node->t.type == NUM) {
                    if (curr.node->t.expr[0] == '-') tot_sgn++;
                    eq.push(curr.node);
                } else {
                    int var_ind = 0;
                    if (curr.node->t.expr[0] == '-') {
                        var_ind = 1;
                        tot_sgn++;
                    }
                    if (std::find(vars.begin(), vars.end(), curr.node->t.expr[var_ind]) == vars.end()) {
                        
                        vars.push_back(curr.node->t.expr[var_ind]);
                    }
                    if ((curr.level % 2) == 0) {
                        cm[curr.node->t.expr[var_ind]].nexp++;
                    } else {
                        cm[curr.node->t.expr[var_ind]].nexp--;
                    }
                }

                tq.pop();
            }
            
            for (int i = 0; i < vars.size(); i++) {
                if (cm[vars[i]].nexp == 0) {

                } else if (cm[vars[i]].nexp == 1) {
                    expr_node* varn = new expr_node;
                    varn->t.expr = vars[i];
                    varn->t.type = VAR;
                    varn->left = nullptr;
                    varn->right = nullptr;
                    eq.push(varn);
                } else {
                    expr_node* numn = new expr_node;
                    numn->t.expr = std::to_string(cm[vars[i]].nexp);
                    numn->t.type = NUM;
                    numn->left = nullptr;
                    numn->right = nullptr;

                    expr_node* varn = new expr_node;
                    varn->t.expr = vars[i];
                    varn->t.type = VAR;
                    varn->left = nullptr;
                    varn->right = nullptr;

                    expr_node* pown = new expr_node;
                    pown->t.expr = '^';
                    pown->t.type = OP;
                    pown->left = varn;
                    varn->parent = pown;
                    pown->right = numn;
                    numn->parent = pown;

                    eq.push(pown);
                }
            }

            if (eq.empty() && !vars.empty()) {
                expr_node* onen = new expr_node;
                onen->t.expr = '1';
                onen->t.type = NUM;
                onen->left = nullptr;
                onen->right = nullptr;

                eq.push(onen);
            }
            if (!eq.empty() && !vars.empty()) {
                expr_node* new_root = gen_rep_expr_tree(eq, '*');

                new_root->parent = root->parent;
                if (root->parent != nullptr) {
                    if (root->parent->left == root) {
                        root->parent->left = new_root;
                        if ((tot_sgn % 2) == 1) expr_tree_apply_neg_impl(new_root, true, LEFT);
                    } else {
                        root->parent->right = new_root;
                        if ((tot_sgn % 2) == 1) expr_tree_apply_neg_impl(new_root, true, RIGHT);
                    }
                } else {
                    if ((tot_sgn % 2) == 1) expr_tree_apply_neg_impl(new_root, true, TREE_BEGIN);
                }
                root = new_root;
            }
        } else {

        }
    } else if (root->t.type == FUNC) {
        resolve_exp_num_den(root->left);
    }
}

expr_node* copy_node(expr_node* node) {
    if (node == nullptr) {
        return nullptr;
    } else {
        expr_node* ret = new expr_node;

        ret->t.expr = node->t.expr;
        ret->t.type = node->t.type;
        ret->left = copy_node(node->left);
        ret->right = copy_node(node->right);
        ret->parent = node->parent;

        return ret;
    }
}

void derivative(expr_node*& root, char wrt) {
    if (root->t.type == NUM) {
        root->t.expr = '0';
    } else if (root->t.type == VAR) {
        if (is_constant(root, wrt)) {
            root->t.expr = '0';
            root->t.type = NUM;
        } else {
            if (root->t.expr[0] == '-') {
                root->t.expr = "-1";
            } else {
                root->t.expr = '1';
            }
            root->t.type = NUM;
            root->left = nullptr;
            root->right = nullptr;
        }
    } else if (root->t.type == OP) {
        if (root->t.expr[0] == '+') {
            derivative(root->left, wrt);
            derivative(root->right, wrt);
        } else if (root->t.expr[0] == '*') {
            bool left_const = is_constant(root->left, wrt);
            bool right_const = is_constant(root->right, wrt);
            if (left_const && right_const) {
                root->t.expr = '0';
                root->t.type = NUM;
                root->right = nullptr;
                root->left = nullptr;
            } else if (left_const && !right_const) {
                derivative(root->right, wrt);
            } else if (!left_const && right_const) {
                derivative(root->left, wrt);
            } else {
                expr_node* ul = copy_node(root->left);
                expr_node* ur = copy_node(root->right);
                expr_node* dl = copy_node(root->left);
                expr_node* dr = copy_node(root->right);
                derivative(dl, wrt);
                derivative(dr, wrt);

                expr_node* mul1 = new expr_node;
                mul1->t.expr = '*';
                mul1->t.type = OP;
                mul1->left = ul;
                ul->parent = mul1;
                mul1->right = dr;
                dr->parent = mul1;
                mul1->parent = root;
                root->left = mul1;

                expr_node* mul2 = new expr_node;
                mul2->t.expr = '*';
                mul2->t.type = OP;
                mul2->left = ur;
                ur->parent = mul2;
                mul2->right = dl;
                dl->parent = mul2;
                mul2->parent = root;
                root->right = mul2;

                root->t.expr = '+';
            }
        } else if (root->t.expr[0] == '/') {
            bool left_const = is_constant(root->left, wrt);
            bool right_const = is_constant(root->right, wrt);
            if (left_const && right_const) {
                root->t.expr = '0';
                root->t.type = NUM;
                root->right = nullptr;
                root->left = nullptr;
            } else if (!left_const && right_const) {
                derivative(root->left, wrt);
            } else {
                expr_node* ul = copy_node(root->left);
                expr_node* ur = copy_node(root->right);
                expr_node* ur2 = copy_node(root->right);
                expr_node* dl = copy_node(root->left);
                expr_node* dr = copy_node(root->right);
                derivative(dl, wrt);
                derivative(dr, wrt);

                expr_node* add = new expr_node;
                add->t.expr = '+';
                add->t.type = OP;
                add->parent = root;

                expr_node* mul1 = new expr_node;
                mul1->t.expr = '*';
                mul1->t.type = OP;
                mul1->left = ur;
                ur->parent = mul1;
                mul1->right = dl;
                dl->parent = mul1;
                mul1->parent = add;
                add->left = mul1;

                expr_node* mul2 = new expr_node;
                mul2->t.expr = '*';
                mul2->t.type = OP;
                mul2->left = ul;
                ul->parent = mul2;
                mul2->right = dr;
                dr->parent = mul2;
                mul2->parent = add;
                add->right = mul2;
                expr_tree_apply_neg_impl(mul2, true, RIGHT);
                root->left = add;

                expr_node* twon = new expr_node;
                twon->t.expr = '2';
                twon->t.type = NUM;
                twon->right = nullptr;
                twon->left = nullptr;

                expr_node* pown = new expr_node;
                pown->t.expr = '^';
                pown->t.type = OP;
                pown->left = ur2;
                ur2->parent = pown;
                pown->right = twon;
                twon->parent = pown;
                pown->parent = root;

                root->right = pown;
            }
        } else {
            bool left_const = is_constant(root->left, wrt);
            bool right_const = is_constant(root->right, wrt);
            if (left_const && right_const) {
                root->t.expr = '0';
                root->t.type = NUM;
                root->right = nullptr;
                root->left = nullptr;
            } else if (!left_const && right_const) {
                expr_node* ur = copy_node(root->right);
                expr_node* ur2 = copy_node(root->right);
                expr_node* ul = copy_node(root->left);
                expr_node* dl = copy_node(root->left);
                derivative(dl, wrt);

                expr_node* mon = new expr_node;
                mon->t.expr = "-1";
                mon->t.type = NUM;
                mon->left = nullptr;
                mon->right = nullptr;

                expr_node* add = new expr_node;
                add->t.expr = '+';
                add->t.type = OP;
                add->left = ur;
                ur->parent = add;
                add->right = mon;
                mon->parent = add;

                expr_node* pown = new expr_node;
                pown->t.expr = '^';
                pown->t.type = OP;
                pown->left = ul;
                ul->parent = pown;
                pown->right = add;
                add->parent = pown;

                expr_node* mul = new expr_node;
                mul->t.expr = '*';
                mul->t.type = OP;
                mul->left = ur2;
                ur2->parent = mul;
                mul->right = pown;
                pown->parent = mul;

                root->t.expr = '*';
                root->right = mul;
                mul->parent = root;
                root->left = dl;
                dl->parent = root;
            } else if (left_const && !right_const) {
                expr_node* ul = copy_node(root->left);
                expr_node* ul2 = copy_node(root->left);
                expr_node* ur = copy_node(root->right);
                expr_node* dr = copy_node(root->right);
                derivative(dr, wrt);
                
                expr_node* logn = new expr_node;
                logn->t.expr = "log";
                logn->t.type = FUNC;
                logn->left = ul2;
                ul2->parent = logn;
                logn->right = logn->left;

                expr_node* mul1 = new expr_node;
                mul1->t.expr = '*';
                mul1->t.type = OP;
                mul1->left = dr;
                dr->parent = mul1;
                mul1->right = logn;
                logn->parent = mul1;

                expr_node* pown = new expr_node;
                pown->t.expr = '^';
                pown->t.type = OP;
                pown->left = ul;
                ul->parent = pown;
                pown->right = ur;
                ur->parent = pown;

                root->t.expr = '*';
                root->left = pown;
                pown->parent = root;
                root->right = mul1;
                mul1->parent = root->right;
            } else {
                expr_node* ur = copy_node(root->right);
                expr_node* ur2 = copy_node(root->right);
                expr_node* ul = copy_node(root->left);
                expr_node* ul2 = copy_node(root->left);
                expr_node* ul3 = copy_node(root->left);
                expr_node* dl = copy_node(root->left);
                expr_node* dr = copy_node(root->right);
                derivative(dl, wrt);
                derivative(dr, wrt);

                expr_node* div = new expr_node;
                div->t.expr = '/';
                div->t.type = OP;
                div->left = ur;
                ur->parent = div;
                div->right = ul;
                ul->parent = div;

                expr_node* mul1 = new expr_node;
                mul1->t.expr = '*';
                mul1->t.type = OP;
                mul1->left = dl;
                dl->parent = mul1;
                mul1->right = div;
                div->parent = mul1;

                expr_node* logn = new expr_node;
                logn->t.expr = "log";
                logn->t.type = FUNC;
                logn->left = ul2;
                ul2->parent = logn;
                logn->right = logn->left;

                expr_node* mul2 = new expr_node;
                mul2->t.expr = '*';
                mul2->t.type = OP;
                mul2->left = dr;
                dr->parent = mul2;
                mul2->right = logn;
                logn->parent = mul2;

                expr_node* add = new expr_node;
                add->t.expr = '+';
                add->t.type = OP;
                add->left = mul2;
                mul2->parent = add;
                add->right = mul1;
                mul1->parent = add;

                expr_node* pown = new expr_node;
                pown->t.expr = '^';
                pown->t.type = OP;
                pown->left = ul3;
                ul3->parent = pown;
                pown->right = ur2;
                ur2->parent = pown;

                root->t.expr = '*';
                root->left = pown;
                pown->parent = root;
                root->right = add;
                add->parent = root;
            }
        }
    } else {
        if (root->t.expr == "sin") {
            expr_node* ua = copy_node(root->left);
            expr_node* da = copy_node(root->left);
            derivative(da, wrt);

            expr_node* cosn = new expr_node;
            cosn->t.expr = "cos";
            cosn->left = ua;
            ua->parent = cosn;
            cosn->right = cosn->left;

            root->t.expr = '*';
            root->t.type = OP;
            root->left = cosn;
            cosn->parent = root;
            root->right = da;
            da->parent = root;
        } else if (root->t.expr == "-sin") {
            expr_node* ua = copy_node(root->left);
            expr_node* da = copy_node(root->left);
            derivative(da, wrt);

            expr_node* cosn = new expr_node;
            cosn->t.expr = "-cos";
            cosn->left = ua;
            ua->parent = cosn;
            cosn->right = cosn->left;

            root->t.expr = '*';
            root->t.type = OP;
            root->left = cosn;
            cosn->parent = root;
            root->right = da;
            da->parent = root;
        } else if (root->t.expr == "cos") {
            expr_node* ua = copy_node(root->left);
            expr_node* da = copy_node(root->left);
            derivative(da, wrt);

            expr_node* cosn = new expr_node;
            cosn->t.expr = "-sin";
            cosn->left = ua;
            ua->parent = cosn;
            cosn->right = cosn->left;

            root->t.expr = '*';
            root->t.type = OP;
            root->left = cosn;
            cosn->parent = root;
            root->right = da;
            da->parent = root;
        } else if (root->t.expr == "-cos") {
            expr_node* ua = copy_node(root->left);
            expr_node* da = copy_node(root->left);
            derivative(da, wrt);

            expr_node* cosn = new expr_node;
            cosn->t.expr = "sin";
            cosn->left = ua;
            ua->parent = cosn;
            cosn->right = cosn->left;

            root->t.expr = '*';
            root->t.type = OP;
            root->left = cosn;
            cosn->parent = root;
            root->right = da;
            da->parent = root;
        } else if (root->t.expr == "exp") {
            expr_node* ua = copy_node(root->left);
            expr_node* da = copy_node(root->left);
            derivative(da, wrt);

            expr_node* cosn = new expr_node;
            cosn->t.expr = "exp";
            cosn->left = ua;
            ua->parent = cosn;
            cosn->right = cosn->left;

            root->t.expr = '*';
            root->t.type = OP;
            root->left = cosn;
            cosn->parent = root;
            root->right = da;
            da->parent = root;
        } else if (root->t.expr == "-exp") {
            expr_node* ua = copy_node(root->left);
            expr_node* da = copy_node(root->left);
            derivative(da, wrt);

            expr_node* cosn = new expr_node;
            cosn->t.expr = "-exp";
            cosn->left = ua;
            ua->parent = cosn;
            cosn->right = cosn->left;

            root->t.expr = '*';
            root->t.type = OP;
            root->left = cosn;
            cosn->parent = root;
            root->right = da;
            da->parent = root;
        } else if (root->t.expr == "log") {
            expr_node* ua = copy_node(root->left);
            expr_node* da = copy_node(root->left);
            derivative(da, wrt);

            root->t.expr = '/';
            root->t.type = OP;
            root->left = da;
            da->parent = root;
            root->right = ua;
            ua->parent = root;
        } else {
            expr_node* ua = copy_node(root->left);
            expr_node* da = copy_node(root->left);
            derivative(da, wrt);

            root->t.expr = '/';
            root->t.type = OP;
            root->left = da;
            da->parent = root;
            root->right = ua;
            ua->parent = root;
            expr_tree_apply_neg_impl(root->left, true, LEFT);
        }
    }
}