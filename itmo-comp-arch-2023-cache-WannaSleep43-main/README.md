# ITMO Computer Architecture 2023 — Модель кэш-памяти на C++

## Описание
Симулятор кэш-памяти для анализа:
- Политик замещения (LRU, FIFO и др.)
- Параметров кэша: размер, ассоциативность, размер блока
- Эффективности работы (hit/miss rate)](https://github.com/WannaSleep43/computer-architecture-course/blob/main/itmo-comp-arch-2023-cache-WannaSleep43-main/assignment.pdf)

## Структура
- `src/` — код модели кэша
- `examples/` — примеры трасс обращений к памяти
- `results/` — примеры результатов симуляции

## Сборка и запуск
```bash
g++ -std=c++17 -o cache_simulator main.cpp
./cache_simulator trace_file.txt
