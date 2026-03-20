# Домашнее задание 02: Разработка REST API сервиса
## MAX Disk 360

### Описание системы

MAX Disk 360 - отечественная система файлохранилища. Состоит из нескольких микросервисов, написана на C++ с использованием фреймворка Poco.

Система MAX Disk 360 предоставляет следующие возможности:
1) Регистрация пользователя
2) Вход пользователя
3) Поиск пользователя по маске имени и фамилии
4) Поиск пользователя по логину
5) Создание новой папки пользователем
6) Получение списка всех папок пользователя
7) Удаление папки пользователя
8) Создание (загрузка) файла в папку
9) Получение содержимого файла пользователя
10) Удаление файла пользователя

### Архитектура системы

Архитектура разработанной системе представлена в папке hw1 репозитория. В рамках данного ДЗ были разработаны следующие сервисы:
- API Gateway
- Identity Service
- File Management Service

Язык сервисов - C++, фреймворк - Poco.

Сервис API Gateway является входной точкой для всех запросов к системе. Он выполняет аутентификацию и авторизацию пользователя, проверяя валидность JWT токена из Authorization HTTP хэдера, и пересылает запрос (вместе с отвалидированным JWT токеном) на необходимый сервис - Identity Service или File Management Service согласно правилам проксирования.
  
Согласно задания, вместо реальных баз данных (PostgreSQL для Identity Service и Mongo DB для File Management Service) хранение данных о пользователях и их папок и файлов организовано посредством In-memory хранилища - хэш-мапы (std::unordered_map).

Кроме этого, в рамках данного задания не был реализован сервис Notification Service - сервис отправки сообщений пользователям через Telegram. Реализация данного сервиса, а также взаимодействия с ним (посредством Kafka) будет в последующем ДЗ. На данный момент, в Identity Service написан интерфейс отправки сообщения в Kafka и его мок-имплементация, выполняющая логирование события, которое должно будет отправляться в Notification Service по Kafka.

### API системы
#### Ссылки на OpenAPI спецификации API сервисов

Полное API всей системы представлено в формате OpenAPI в [файле](openapi.yaml).

Полное API конкретных сервисов, на которые API Gateway пересылает запросы, представлено в формате OpenAPI в файлах:
* [Identity Service](identity-service/openapi.yaml)
* [File Management Service](file-management-service/openapi.yaml)

#### Примеры запросов/ответов

Базовый URL для всех запросов: http://localhost:8080/max-disk

##### Аутентификация и регистрация

###### Регистрация пользователя
Запрос:
```
curl -X POST http://localhost:8080/max-disk/v1/users/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "phone": "+78005553535",
    "email": "ivan.petrov@max.ru",
    "firstName": "Иван",
    "lastName": "Петров"
  }'
```
Успешный ответ (HTTP 201) - ID пользователя:
```
{
    "id": "74ea475d-632d-4728-84b4-2f1c28100ee1"
}
```
###### Вход пользователя
Запрос:
```
curl -X POST http://localhost:8080/max-disk/v1/users/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "login": "ivan.petrov@max.ru",
    "password": "password"
  }'
```
Успешный ответ (HTTP 200) - JWT Access токен для доступа к методам системы
```
{
    "accessToken": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VySWQiOiI1NTBlODQwMC1lMjliLTQxZDQtYTcxNi00NDY2NTU0NDAwMDAiLCJyb2xlIjoidXNlciIsImV4cCI6MTcwNTQ5NjMwMn0.abc123"
}
```
Ошибочный ответ - неверный пароль (HTTP 401)
```
{
    "error": "Invalid login or password"
}
```
##### Поиск пользователей
###### Поиск пользователя по маске имени и фамилии
Запрос:
```
curl -X GET "http://localhost:8080/max-disk/v1/users/search?firstName=И*&lastName=*ов*" \
  -H "Authorization: Bearer $TOKEN"
```
Успешный ответ (HTTP 200):
```
{
    "users": [
        "{\"firstName\":\"Иван\",\"id\":\"74ea475d-632d-4728-84b4-2f1c28100ee1\",\"lastName\":\"Петров\",\"login\":\"ivan.petrov@max.ru\"}"
    ]
}
```
В методах поиска пользователей поддерживается маска * в начале или конце строки. Например:
* ```firstName=*Иван*``` — найдёт пользователей с именами, содержащие "Иван"
* ```firstName=Иван*``` — найдёт пользователей с именами, начинающиеся с "Иван"
* ```lastName=*ов``` — найдёт пользователей с фамилиями, заканчивающиеся на "ов"
###### Поиск пользователя по логину
Запрос:
```
curl -X GET "http://localhost:8080/max-disk/v1/users/search/login?login=ivan.petrov@max.ru" \
  -H "Authorization: Bearer $TOKEN"
```
Успешный ответ (HTTP 200):
```
{
    "firstName": "Иван",
    "id": "74ea475d-632d-4728-84b4-2f1c28100ee1",
    "lastName": "Петров",
    "login": "ivan.petrov@max.ru"
}
```
##### Работа с папками
###### Создание новой папки пользователем
Запрос:
```
curl -X POST http://localhost:8080/max-disk/v1/folders \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "Документы"
  }'
```
Успешный ответ (HTTP 201):
```
{
    "id": "332f96df-5a87-43e3-932b-8e4fe82e9dc9"
}
```
###### Получение списка всех папок пользователя
Запрос:
```
curl -X GET http://localhost:8080/max-disk/v1/folders \
  -H "Authorization: Bearer $TOKEN"
```
Успешный ответ (HTTP 200):
```
{
    "folders": [
        "{\"createdAt\":\"2026-03-20T20:41:23Z\",\"id\":\"332f96df-5a87-43e3-932b-8e4fe82e9dc9\",\"name\":\"Тестовая папка\"}"
    ]
}
```

###### Удаление папки пользователя
Запрос:
```
FOLDER_ID="332f96df-5a87-43e3-932b-8e4fe82e9dc9"

curl -X DELETE "http://localhost:8080/max-disk/v1/folders/$FOLDER_ID" \
  -H "Authorization: Bearer $TOKEN"
```
Успешный ответ (HTTP 204):
*пустое тело*

##### Работа с файлами
###### Создание (загрузка) файла в папку
Запрос:
```
FOLDER_ID="332f96df-5a87-43e3-932b-8e4fe82e9dc9"

curl -X POST "http://localhost:8080/max-disk/v1/folders/$FOLDER_ID/files" \
-H "Authorization: Bearer $TOKEN" \
-H 'Content-Type: multipart/form-data' \
--form 'file=@"/Users/lantum/Documents/study/software-engineering-homeworks/hw2/openapi.yaml"'
```
Успешный ответ (HTTP 201):
```
{
    "id": "e5ece7e3-398c-44ff-892a-a968da6f46ae"
}
```
###### Получение содержимого файла пользователя
Запрос:
```
FOLDER_ID="332f96df-5a87-43e3-932b-8e4fe82e9dc9"

curl -X GET "http://localhost:8080/max-disk/v1/folders/$FOLDER_ID/files?name=openapi.yaml" \
  -H "Authorization: Bearer $TOKEN"
```
Успешный ответ (HTTP 200):
*содержимое файла*

###### Удаление файла пользователя
Запрос:
```
FOLDER_ID="332f96df-5a87-43e3-932b-8e4fe82e9dc9"
FILE_ID="e5ece7e3-398c-44ff-892a-a968da6f46ae"

curl -X DELETE "http://localhost:8080/max-disk/v1/folders/$FOLDER_ID/files/$FILE_ID" \
  -H "Authorization: Bearer $TOKEN"
```
Успешный ответ (HTTP 204):
*пустое тело*

### Сборка и запуск

Сборка сервисов происходит с использованием Docker'а и Docker-образов.
Для запуска необходимо иметь установленный **Docker демон** и **Docker Compose утилиту**.

Чтобы запустить сервисы, достаточно выполнить следующие команды в терминале:
```
git clone https://github.com/lantum1/software-engineering-homeworks.git
cd software-engineering-homeworks/hw2
docker compose up
```

Методы системы будут доступны на локальном хосте (localhost) и порту 8080.

### Запуск тестов

Для данной системы были написаны unit и интеграционные тесты.

Интеграционные тесты представлены в виде [Postman коллекции](<MAX Disk 360 - Интеграционные тесты.postman_collection.json>) - они покрывают некоторые основные методы системы, проверяют аутентификацию и авторизацию API Gateway'я.

Для запуска интеграционных тестов, достаточно импортировать Postman коллекцию в Postman и нажать на кнопку запуска скриптов.

**Важно:** для работы интеграционных тестов необходимо, чтобы вся система была локально поднята (см. Сборка и запуск). Кроме этого, интеграционные тесты модифицируют состояние репозиториев поднятой локально системы, поэтому они могут отработать только один раз до момента остановки Docker контейнеров сервисов системы.

Unit-тесты написаны внутри Identity Service и File Management Service с использованием gtest и проверяют методы классов сервисного уровня.

Для запуска unit-тестов, достаточно запустить [скрипт](run_unit_tests.sh) - данный скрипт поднимет Docker контейнеры с тестами каждого сервиса.