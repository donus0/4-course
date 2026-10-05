#include "lexer.hpp"

#include <cctype>
#include <stdexcept>

static bool is_letter(char c) {
    return std::isalpha(static_cast<unsigned char>(c));
}

static bool is_digit(char c) {
    return std::isdigit(static_cast<unsigned char>(c));
}

static bool is_name_char(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

static bool is_not_semicolon(char c) {
    return c != ';';
}

lexer::lexer(const std::string& source) : _source(source), _pos(0), _line(1) {}

// Текущий символ или '\0', если дошли до конца текста
char lexer::current() const {
    if (_pos < _source.size()) {
        return _source[_pos];
    }
    return '\0';
}

void lexer::error(const std::string& message) const {
    throw std::runtime_error("Синтаксическая ошибка (строка " + std::to_string(_line) + "): " + message);
}

void lexer::skip_spaces() {
    while (std::isspace(static_cast<unsigned char>(current()))) {
        if (current() == '\n') {
            _line++;
        }
        _pos++;
    }
}

std::string lexer::read_while(bool (*condition)(char)) {
    std::size_t start = _pos;
    while (_pos < _source.size() && condition(_source[_pos])) {
        if (_source[_pos] == '\n') {
            _line++;
        }
        _pos++;
    }
    return _source.substr(start, _pos - start);
}

std::string lexer::read_op() {
    std::string result = read_while(is_letter);
    if (result.empty()) {
        error("ожидалось имя операции (set sum min mul div mod and or xor not shl shr cmp exp inp out)");
    }
    return result;
}

std::string lexer::read_arg() {
    char c = current();

    if (c == '-' || is_digit(c)) {
        std::string sign;
        if (c == '-') {
            sign = "-";
            _pos++;
        }
        std::string digits = read_while(is_digit);
        if (digits.empty()) {
            error("ожидалась цифра после знака '-'");
        }
        return sign + digits;
    }

    if (is_letter(c) || c == '_') {
        return read_while(is_name_char);
    }

    error("ожидался аргумент (целое число или имя переменной)");
    return "";
}

std::string lexer::read_expr() {
    std::string result = read_while(is_not_semicolon);
    while (!result.empty() && std::isspace(static_cast<unsigned char>(result.back()))) {
        result.pop_back();
    }
    if (result.empty()) {
        error("ожидалось выражение");
    }
    return result;
}

bool lexer::next(statement& st) {
    skip_spaces();
    if (_pos >= _source.size()) {
        return false;
    }

    st.args.clear();
    st.line = _line;
    st.op = read_op();

    if (current() != ' ' && current() != '\t') {
        error("после операции ожидался пробел");
    }
    skip_spaces();

    if (current() != ';') {
        st.args.push_back(read_arg());
        skip_spaces();

        while (current() == ',') {
            _pos++;
            skip_spaces();
            if (st.op == "exp" && st.args.size() == 1) {
                st.args.push_back(read_expr());
            } else {
                st.args.push_back(read_arg());
            }
            skip_spaces();
        }
    }

    if (current() != ';') {
        error("ожидался символ ';'");
    }
    _pos++;

    return true;
}
