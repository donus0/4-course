#pragma once

#include <string>
#include <vector>

// Место в тексте программы: смещение от начала файла и номер строки
struct position {
    std::size_t pos = 0;
    int line = 1;
};

// Одна команда программы, например "sum a,5;" -> op = "sum", args = {"a", "5"}
// Заголовки блоков тоже считаются командами:
//   "if a {"    -> op = "if",    args = {"a"}
//   "while a {" -> op = "while", args = {"a"}
//   "else {"    -> op = "else",  args = {}
//   "}"         -> op = "}",     args = {}
struct statement {
    std::string op;
    std::vector<std::string> args;
    int line;
    std::size_t pos = 0;  // смещение начала команды в тексте
};

class lexer {
private:
    const std::string& _source;
    std::size_t _pos;
    int _line;

    char current() const;
    void error(const std::string& message) const;
    void skip_spaces();
    std::string read_while(bool (*condition)(char));
    std::string read_op();
    std::string read_arg();
    std::string read_expr();
    void expect_space();
    void expect_open_brace();

public:
    lexer(const std::string& source);

    // Читает следующую команду в st. Возвращает false, если программа закончилась.
    bool next(statement& st);

    // Текущее место в тексте и переход (прыжок) в другое место
    position where() const;
    void jump(const position& to);
};
