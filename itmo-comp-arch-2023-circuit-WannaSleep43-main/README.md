# ITMO Computer Architecture 2023 — Цифровой стек на Verilog и Logisim

## Описание
Учебный проект по курсу "Архитектура компьютеров". Реализация структуры стека с помощью цифровой схемы:
- **Модель на Logisim** для визуального моделирования
- **Verilog-описание схемы** для более глубокой аппаратной симуляции

## Структура
- `logisim/` — схема стека для Logisim
- `verilog/` — Verilog-модули, описание поведения и тестбенчи
- `README.md` — текущее описание

## Зависимости
- [Logisim Evolution](https://github.com/reds-heig/logisim-evolution) (для схем)
- Icarus Verilog или другой симулятор Verilog

## Функции стека
- Операции PUSH и POP
- Обработка переполнения и опустошения
- Реализация на триггерах и регистрах

## Проверка verilog локально (из корня репозитория):

1. `iverilog -g2012 -o stack_tb.out stack_behaviour_tb.sv`
2. `vvp stack_tb.out +TIMES=3 +OUTCSV=st_stack_3.csv`
3. `python st_stack_3.csv .github/workflows/ref_stack_3.csv`

Также посмотреть логи можно в файле `st_stack_3.csv`. Проверяем значения на выходе только при CLK=1.

> Примечание: вместо `edge signal` используйте `signal`

## Проверка logisim локально (из корня репозитория):

WIP
