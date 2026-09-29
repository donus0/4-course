#include "lexer.hpp"

#include <cctype>
#include <stdexcept>

lexer::lexer(const std::string& source) {
    _source = source;
    _pos = 0;
    _line = 1;
}

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

// Имя операции: set, sum, min, ...
std::string lexer::read_op() {
    std::string result;
    while (std::isalpha(static_cast<unsigned char>(current()))) {
        result += current();
        _pos++;
    }
    if (result.empty()) {
        error("ожидалось имя операции (set sum min mul div mod inp out)");
    }
    return result;
}

// Аргумент: целое число (возможно, со знаком минус) или имя переменной
std::string lexer::read_arg() {
    std::string result;
    char c = current();

    if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
        if (c == '-') {
            result += '-';
            _pos++;
        }
        if (!std::isdigit(static_cast<unsigned char>(current()))) {
            error("ожидалась цифра после знака '-'");
        }
        while (std::isdigit(static_cast<unsigned char>(current()))) {
            result += current();
            _pos++;
        }
        return result;
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        while (std::isalnum(static_cast<unsigned char>(current())) || current() == '_') {
            result += current();
            _pos++;
        }
        return result;
    }

    error("ожидался аргумент (целое число или имя переменной)");
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

    // Читаем аргументы через запятую, пока не встретим ';'
    if (current() != ';') {
        st.args.push_back(read_arg());
        skip_spaces();

        while (current() == ',') {
            _pos++;
            skip_spaces();
            st.args.push_back(read_arg());
            skip_spaces();
        }
    }

    if (current() != ';') {
        error("ожидался символ ';'");
    }
    _pos++;

    return true;
}
