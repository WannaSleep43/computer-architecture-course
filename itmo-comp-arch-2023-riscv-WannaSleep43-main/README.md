# ITMO Computer Architecture 2023 — Дизассемблер и анализатор инструкций RISC-V

# disassembler
Решение лабораторной работы №4 "ISA" (Архитектура ЭВМ, 1 курс, КТ ИТМО)
## Описание условия
* Дан elf-файл 
* Используется ISA RISC-V, а именно наборы RV32I, RV32M
* Нужно написать транслятор этого машинного кода в текст программы на ассемблере (дизассемблер)
* Обрабатывать нужно только две секции: text и symtab

С полным условием можно ознакомиться в файле [assignment.pdf](itmo-comp-arch-2023-riscv-WannaSleep43-main/assignment.pdf)

## Структура
- `src/` — исходники анализатора
- `tests/` — бинарные образы для тестирования
- `README.md` — документация

## Сборка и запуск
```bash
g++ -std=c++17 -o riscv_disasm main.cpp
./riscv_disasm binary_input_file
