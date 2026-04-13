# Домашнее задание 04: Проектирование и работа с MongoDB

## Описание изменений

В рамках архитектуры, схема которой представлена в hw1 папке, NoSQL БД MongoDB заложена в одном сервисе - File Management Service.

В рамках выполнения данной ЛР была спроектирована БД с именем **file-management** для семантики сервиса File Management Service - хранение информации и файлах и папках пользователей, а также хранение бинарного содержимого файлов.\
Кроме этого, был написан [data.js](file-management-service/db/data.js) - скрипт по предварительному наполнению БД тестовыми данными, [queries.js](file-management-service/db/queries.js) - скрипт с тестовыми различными запросами, включающие различные команды, операнды MongoDB и аггрегирующие пайплайны и [validation.js](file-management-service/db/validation.js) - скрипт для добавления валидации структуры сохраняемых документов.

Кроме этого, была выполнена интеграция созданной MongoDB с уже существующим сервисом File Management Service.

Остальные сервисы и контейнеры (Identity Service с PostgreSQL БД, API Gateway, БД PostgreSQL для Notification Service) были взяты из предыдущей ЛР - из hw3 папки.

## Описание схем БД
### File Management Service

Описание проектирования схемы БД **file-management** (разделение на коллекции, структура документов, обоснование индексов, обоснование выбора embedded/reference) для данного сервиса расположено в файле [schema_design.md](file-management-service/db/schema_design.md).

## Скрипты с демонстрацией работы с БД

**Скрипт, добавляющий валидацию на документы коллекций:** [validation.js](file-management-service/db/validation.js)\
**Скрипт со вставкой тестовых данных в коллекции:** [data.js](file-management-service/db/data.js)\
**Скрипт различных запросов с документами коллекций:** [queries.js](file-management-service/db/queries.js)

## Инструкции по запуску

MongoDB добавлена в [docker-compose.yaml](docker-compose.yaml). Ее первичная инициализация (создание схемы, добавление валидации документов и наполнение тестовыми данными) происходит при первичном старте Docker контейнера. 

Для поднятия всей системы, в том числе MongoDB, с которой сынтегрирован File Management Service, достаточно выполнить следующие команды в терминале:
```
git clone https://github.com/lantum1/software-engineering-homeworks.git
cd software-engineering-homeworks/hw4
docker compose up
```