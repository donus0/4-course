# Постфиксное преобразование выражений (метод Дийкстры)

Реализация на C++:

- [Stack.hpp](include/Stack.hpp) — стек на базе односвязного списка (шаблонный класс).
- [InfixToPostfix.cpp](src/InfixToPostfix.cpp) — перевод инфиксного выражения в постфиксное по алгоритму Дийкстры (сортировочная станция). Числа однозначные, поддерживаются `+ - * / ^` и скобки.
- [PostfixEval.cpp](src/PostfixEval.cpp) — вычисление постфиксного выражения. Числа могут быть многозначными (в т.ч. дробными), токены разделяются пробелом.
- [main.cpp](src/main.cpp) — консольное меню для проверки обеих частей.

## Требования

- Windows 11 + [MSYS2](https://www.msys2.org/), установленный в стандартный путь `C:\msys64`.
- Окружение **UCRT64** с установленным toolchain (терминал "MSYS2 UCRT64"):

```bash
pacman -S mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-gdb
```

- В VS Code установлено расширение **C/C++** (ms-vscode.cpptools).

Если MSYS2 установлен в другой путь, поправьте пути к `g++.exe` / `gdb.exe` в файлах `.vscode/tasks.json`, `.vscode/launch.json` и `.vscode/c_cpp_properties.json`.

## Сборка и запуск

Откройте папку `postfix` в VS Code и нажмите **Ctrl+Shift+B** (задача *g++ build postfix (MSYS2)*) — соберётся `bin/postfix.exe`.

Для запуска с отладкой — **F5** (конфигурация *g++ build and debug postfix (MSYS2 gdb)*), она сама пересоберёт проект перед запуском.

Сборка вручную из терминала MSYS2 UCRT64:

```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/main.cpp src/InfixToPostfix.cpp src/PostfixEval.cpp -o bin/postfix.exe
./bin/postfix.exe
```

## Пример работы

```
Выбор: 1
Введите инфиксное выражение: (1+2)*3-9/3^2
Постфиксная форма: 1 2 + 3 * 9 3 2 ^ / -

Выбор: 2
Введите постфиксное выражение (токены через пробел): 12 3 4 * +
Результат: 24
```
