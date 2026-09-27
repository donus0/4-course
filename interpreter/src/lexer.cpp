#include "lexer.hpp"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

static char peek(const std::string& s, std::size_t pos) {
    return pos < s.size() ? s[pos] : '\0';
}

static void fail(int line, const std::string& message) {
    throw std::runtime_error("Синтаксическая ошибка (строка " + std::to_string(line) + "): " + message);
}

static void skip_whitespace(const std::string& s, std::size_t& pos, int& line) {
    while (pos < s.size()) {
        char c = s[pos];
        if (c == '\n') {
            ++line;
            ++pos;
        } else if (std::isspace(static_cast<unsigned char>(c))) {
            ++pos;
        } else {
            break;
        }
    }
}

static std::string parse_op(const std::string& s, std::size_t& pos, int line) {
    std::string result;
    while (std::isalpha(static_cast<unsigned char>(peek(s, pos)))) {
        result += s[pos];
        ++pos;
    }
    if (result.empty()) {
        fail(line, "ожидалось имя операции (set sum min mul div mod inp out)");
    }
    return result;
}

static std::string parse_arg(const std::string& s, std::size_t& pos, int line) {
    std::string result;
    char c = peek(s, pos);

    if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
        if (c == '-') {
            result += '-';
            ++pos;
            c = peek(s, pos);
        }
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            fail(line, "ожидалась цифра после знака '-'");
        }
        while (std::isdigit(static_cast<unsigned char>(peek(s, pos)))) {
            result += s[pos];
            ++pos;
        }
        return result;
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        while (std::isalnum(static_cast<unsigned char>(peek(s, pos))) || peek(s, pos) == '_') {
            result += s[pos];
            ++pos;
        }
        return result;
    }

    fail(line, "ожидался аргумент (целое число или имя переменной)");
}

bool next_statement(const std::string& source, std::size_t& pos, int& line, statement& out) {
    skip_whitespace(source, pos, line);
    if (pos >= source.size()) {
        return false;
    }

    out.args.clear();
    out.line = line;
    out.op = parse_op(source, pos, line);

    char after_op = peek(source, pos);
    if (after_op != ' ' && after_op != '\t') {
        fail(line, "после операции ожидался пробел");
    }
    skip_whitespace(source, pos, line);

    if (peek(source, pos) != ';') {
        while (true) {
            out.args.push_back(parse_arg(source, pos, line));
            skip_whitespace(source, pos, line);

            if (peek(source, pos) == ',') {
                ++pos;
                skip_whitespace(source, pos, line);
                continue;
            }
            break;
        }
    }

    if (peek(source, pos) != ';') {
        fail(line, "ожидался символ ';'");
    }
    ++pos;

    return true;
}

std::string read_file(const char* path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error(std::string("не удалось открыть файл: ") + path);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
