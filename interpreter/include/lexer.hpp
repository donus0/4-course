#pragma once

#include <string>
#include <vector>

struct statement {
    std::string op;
    std::vector<std::string> args;
    int line;
};

bool next_statement(const std::string& source, std::size_t& pos, int& line, statement& out);

std::string read_file(const char* path);
