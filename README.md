# C hiddenfile module

### About

Модуль для ядра linux, который регистрирует девайс, доступный только по паролю
Создан для демонстрации возомжности прятать файл с помощью модификации ядра linux

### Quickstart

Загрузка модуля:

```bash
$ make load
```

Выгрузка модуля:

```bash
$ make unload
```

Получение логов ядра:

```bash
$ ./live_logs.sh
```

Создание девайса hiddenfile, где [MAJOR] - полученное в логах зачение major

```bash
$ sudo mknod hiddenfile c [MAJOR] 0
```
