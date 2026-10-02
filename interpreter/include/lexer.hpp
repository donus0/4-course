#pragma once

#include <string>
#include <vector>

// Одна команда программы, например "sum a,5;" -> op = "sum", args = {"a", "5"}
struct statement {
    std::string op;
    std::vector<std::string> args;
    int line;
};

// Разбирает текст программы на команды по одной
class lexer {
private:
    std::string _source;
    std::size_t _pos;
    int _line;

    char current() const;
    void error(const std::string& message) const;
    void skip_spaces();
    std::string read_op();
    std::string read_arg();
    std::string read_expr();

public:
    lexer(const std::string& source);

    // Читает следующую команду в st. Возвращает false, если программа закончилась.
    bool next(statement& st);
};
