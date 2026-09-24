#include <cctype>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "BinaryTree.hpp"

namespace {

// Символ считается частью слова, если это ASCII-буква/цифра, либо байт
// многобайтовой UTF-8 последовательности (например, кириллица) — так слова
// на кириллице не разбиваются посимвольно.
bool isWordChar(unsigned char c) {
    return std::isalnum(c) != 0 || c >= 0x80;
}

// Приводит слово к нижнему регистру: ASCII — через tolower, кириллицу (UTF-8,
// 2-байтовые последовательности U+0400-U+04FF) — вручную, т.к. tolower/locale
// с многобайтовой UTF-8 кириллицей не работает без подключения <locale>.
std::string toLowerWord(const std::string& word) {
    std::string result;
    result.reserve(word.size());

    std::size_t i = 0;
    while (i < word.size()) {
        unsigned char b0 = static_cast<unsigned char>(word[i]);
        if (b0 < 0x80) {
            result += static_cast<char>(std::tolower(b0));
            ++i;
        } else if ((b0 & 0xE0) == 0xC0 && i + 1 < word.size()) {
            unsigned char b1 = static_cast<unsigned char>(word[i + 1]);
            unsigned int codepoint = ((b0 & 0x1Fu) << 6) | (b1 & 0x3Fu);
            if (codepoint == 0x0401) {
                codepoint = 0x0451;  // Ё -> ё
            } else if (codepoint >= 0x0410 && codepoint <= 0x042F) {
                codepoint += 0x20;  // А-Я -> а-я
            }
            result += static_cast<char>(0xC0 | (codepoint >> 6));
            result += static_cast<char>(0x80 | (codepoint & 0x3Fu));
            i += 2;
        } else {
            // Прочие многобайтовые последовательности копируем без изменений.
            result += static_cast<char>(b0);
            ++i;
        }
    }

    return result;
}

}  // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::string path;
    if (argc > 1) {
        path = argv[1];
    } else {
        std::cout << "Путь к текстовому файлу (пусто — sample.txt): ";
        std::getline(std::cin, path);
        if (path.empty()) {
            path = "sample.txt";
        }
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cout << "Ошибка: не удалось открыть файл \"" << path << "\"\n";
        return 1;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();

    // Дерево строит статистику: ключ — слово, значение — количество вхождений.
    BinaryTree<std::string, int> wordCounts;
    std::size_t totalWords = 0;

    std::size_t i = 0;
    while (i < text.size()) {
        if (!isWordChar(static_cast<unsigned char>(text[i]))) {
            ++i;
            continue;
        }
        std::size_t start = i;
        while (i < text.size() && isWordChar(static_cast<unsigned char>(text[i]))) {
            ++i;
        }
        std::string word = toLowerWord(text.substr(start, i - start));
        wordCounts[word] += 1;  // поиск существующего узла и вставка нового, если его не было
        ++totalWords;
    }

    std::cout << "\nВсего слов: " << totalWords << "\n";
    std::cout << "Уникальных слов: " << wordCounts.size() << "\n\n";
    std::cout << "Частота слов (по алфавиту):\n";
    wordCounts.inorderTraversal([](const std::string& word, const int& count) {
        std::cout << "  " << word << ": " << count << "\n";
    });

    std::cout << "\nНайти слово (поиск в дереве, пусто — пропустить): ";
    std::string query;
    std::getline(std::cin, query);
    if (!query.empty()) {
        std::string normalized = toLowerWord(query);
        const int* count = wordCounts.find(normalized);
        if (count != nullptr) {
            std::cout << "\"" << normalized << "\" встречается " << *count << " раз\n";
        } else {
            std::cout << "\"" << normalized << "\" в тексте не найдено\n";
        }
    }

    return 0;
}
