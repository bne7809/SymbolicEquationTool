#pragma once

#include "print_util.h"
#include <cmath>

float bin_symb_op(float left, float right, char op);
float symb_func(float arg, const std::string& func);
float calc_expr(const std::string& expr);