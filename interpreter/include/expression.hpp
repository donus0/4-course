#pragma once

#include <string>
#include <vector>

#include "lexer.hpp"

std::vector<std::string> infix_to_postfix(const std::string& expr, int line, bool trace = false);

std::string postfix_to_string(const std::vector<std::string>& postfix);

std::vector<statement> postfix_to_commands(const std::vector<std::string>& postfix, const std::string& dst, int line);
