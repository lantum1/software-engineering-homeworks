# Запросы к PostgreSQL БД

## Запросы, подлежащие оптимизации

Список запросов из [queries.sql](queries.sql) файла, на план запросов которых будем смотреть и которые будем отпимизировать:

```
SELECT id, password_hash, is_active FROM identity.users_auth WHERE login = 'ivan.petrov' LIMIT 1;

SELECT login, password_hash, is_active FROM identity.users_auth WHERE id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890' LIMIT 1;

SELECT EXISTS(SELECT 1 FROM identity.users_auth WHERE login = 'ivan.petrov');

SELECT user_id, phone, first_name, last_name, role FROM identity.users_profile WHERE email = 'ivan.petrov@test.com' LIMIT 1;

SELECT email, phone, first_name, last_name, role FROM identity.users_profile WHERE user_id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890' LIMIT 1;

SELECT a.id, a.login, p.email, p.phone, p.first_name, p.last_name, p.role FROM identity.users_profile p JOIN identity.users_auth a ON p.user_id = a.id WHERE p.first_name ILIKE '%Ivan%' AND p.last_name ILIKE '%Petrov%' LIMIT 100;

SELECT EXISTS(SELECT 1 FROM identity.users_profile WHERE (email = 'ivan.petrov@test.com' OR phone = '+79991234567'));

INSERT INTO identity.users_auth (login, password_hash) VALUES ('ivan.petrov', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4.G.2f3f3f3f3f3f');

INSERT INTO identity.users_profile (user_id, email, phone, first_name, last_name) VALUES ('a1b2c3d4-e5f6-7890-abcd-ef1234567890', 'ivan.petrov@test.com', '+79991234567', 'Ivan', 'Petrov');
```

## Подготовка БД перед просмотром статистики

Перед просмотром статистики выполним подготовку БД - добавим в таблицы тестовые данные.\
Перед анализом через EXPLAIN я запустил скрипт, выполнивший вставку 10001 строку в каждую из двух таблиц - users_auth и users_profile.\
Кроме этого, я вставил данные из [data.sql](data.sql) - так как сгенерированные 10001 строка являются около случайным наборот данных, то я решил дополнительно вставить заранее определнный набор данных и составить запросы на данных из этого набора.

Итоговое кол-во строк в таблице **users_auth**:
```
users_auth	10012
```

Итоговое кол-во строк в таблице **users_profile**:
```
users_profile	10012
```

## Просмотр планов запросов до оптимизаций

1. 
```
EXPLAIN SELECT id, password_hash, is_active FROM identity.users_auth WHERE login = 'ivan.petrov' LIMIT 1;
```
```
Limit  (cost=0.29..8.30 rows=1 width=130)
  ->  Index Scan using users_auth_login_key on users_auth  (cost=0.29..8.30 rows=1 width=130)
        Index Cond: ((login)::text = 'ivan.petrov'::text)
```

2. 
```
EXPLAIN SELECT login, password_hash, is_active FROM identity.users_auth WHERE id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890' LIMIT 1;
```
```
Limit  (cost=0.29..8.30 rows=1 width=130)
  ->  Index Scan using users_auth_pkey on users_auth  (cost=0.29..8.30 rows=1 width=130)
        Index Cond: (id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890'::uuid)
```

3. 
```
EXPLAIN SELECT EXISTS(SELECT 1 FROM identity.users_auth WHERE login = 'ivan.petrov');
```
```
Result  (cost=8.30..8.31 rows=1 width=1)
  InitPlan 1 (returns $0)
    ->  Index Only Scan using users_auth_login_key on users_auth  (cost=0.29..8.30 rows=1 width=0)
          Index Cond: (login = 'ivan.petrov'::text)
```

4. 
```
EXPLAIN SELECT user_id, phone, first_name, last_name, role FROM identity.users_profile WHERE email = 'ivan.petrov@test.com' LIMIT 1;
```
```
Limit  (cost=0.29..8.30 rows=1 width=91)
  ->  Index Scan using users_profile_email_key on users_profile  (cost=0.29..8.30 rows=1 width=91)
        Index Cond: ((email)::text = 'ivan.petrov@test.com'::text)
```

5. 
```
EXPLAIN SELECT email, phone, first_name, last_name, role FROM identity.users_profile WHERE user_id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890' LIMIT 1;
```
```
Limit  (cost=0.29..8.30 rows=1 width=91)
  ->  Index Scan using users_profile_pkey on users_profile  (cost=0.29..8.30 rows=1 width=91)
        Index Cond: (user_id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890'::uuid)
```

6. 
```
EXPLAIN SELECT a.id, a.login, p.email, p.phone, p.first_name, p.last_name, p.role FROM identity.users_profile p JOIN identity.users_auth a ON p.user_id = a.id WHERE p.first_name ILIKE '%Ivan%' AND p.last_name ILIKE '%Petrov%' LIMIT 100;
```
```
Limit  (cost=0.29..303.37 rows=1 width=103)
  ->  Nested Loop  (cost=0.29..303.37 rows=1 width=103)
        ->  Seq Scan on users_profile p  (cost=0.00..295.06 rows=1 width=75)
              Filter: (((first_name)::text ~~* '%Ivan%'::text) AND ((last_name)::text ~~* '%Petrov%'::text))
        ->  Index Scan using users_auth_pkey on users_auth a  (cost=0.29..8.30 rows=1 width=44)
              Index Cond: (id = p.user_id)
```

7. 
```
EXPLAIN SELECT EXISTS(SELECT 1 FROM identity.users_profile WHERE (email = 'ivan.petrov@test.com' OR phone = '+79991234567'));
```
```
Result  (cost=12.25..12.26 rows=1 width=1)
  InitPlan 1 (returns $0)
    ->  Bitmap Heap Scan on users_profile  (cost=8.59..15.91 rows=2 width=0)
          Recheck Cond: (((email)::text = 'ivan.petrov@test.com'::text) OR ((phone)::text = '+79991234567'::text))
          ->  BitmapOr  (cost=8.59..8.59 rows=2 width=0)
                ->  Bitmap Index Scan on users_profile_email_key  (cost=0.00..4.29 rows=1 width=0)
                      Index Cond: ((email)::text = 'ivan.petrov@test.com'::text)
                ->  Bitmap Index Scan on users_profile_phone_key  (cost=0.00..4.29 rows=1 width=0)
                      Index Cond: ((phone)::text = '+79991234567'::text)
```

8. 
```
EXPLAIN INSERT INTO identity.users_auth (login, password_hash) VALUES ('ivan.petrov', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4.G.2f3f3f3f3f3f');
```
```
Insert on users_auth  (cost=0.00..0.01 rows=0 width=0)
  ->  Result  (cost=0.00..0.01 rows=1 width=1073)
```

9. 
```
EXPLAIN INSERT INTO identity.users_profile (user_id, email, phone, first_name, last_name) VALUES ('a1b2c3d4-e5f6-7890-abcd-ef1234567890', 'ivan.petrov@test.com', '+79991234567', 'Ivan', 'Petrov');
```
```
Insert on users_profile  (cost=0.00..0.01 rows=0 width=0)
  ->  Result  (cost=0.00..0.01 rows=1 width=846)
```
## Рассуждения об оптимизациях

Почти все SELEСT запросы, выполняемые сервисом, **уже используют индексы** - которые уже создал сам PostgreSQL. Ведь PostgreSQL по умолчанию создает индексы на поля таблицы, имеющие UNIQUE констрейнт (в том числе и поля, помеченные как Primary Key), а в запросах у меня часто идет фильтрация по UNIQUE полям.

**Запрос 7** уже использует индексы и оптимизатор применяет **Index Bitmap Scan** - потому что запрос включает в себя OR.

Точно необходимо добавить индексы, позволяющие **оптимизировать запрос 6** - чтобы быстрее работал поиск ILIKE. Для этого не подойдут hash или b-tree индексы - будем использовать **GIN**.

Попробуем добавить следующие GIN индексы:
```
CREATE INDEX IF NOT EXISTS idx_users_profile_first_name_trgm ON identity.users_profile USING gin (first_name gin_trgm_ops);
CREATE INDEX IF NOT EXISTS idx_users_profile_last_name_trgm ON identity.users_profile USING gin (last_name gin_trgm_ops);
```

Посмотрим на план 6 запроса после добавления индексов:
```
Limit  (cost=12.29..24.32 rows=1 width=103)
  ->  Nested Loop  (cost=12.29..24.32 rows=1 width=103)
        ->  Bitmap Heap Scan on users_profile p  (cost=12.00..16.02 rows=1 width=75)
              Recheck Cond: ((first_name)::text ~~* '%Ivan%'::text)
              Filter: ((last_name)::text ~~* '%Petrov%'::text)
              ->  Bitmap Index Scan on idx_users_profile_first_name_trgm  (cost=0.00..12.00 rows=1 width=0)
                    Index Cond: ((first_name)::text ~~* '%Ivan%'::text)
        ->  Index Scan using users_auth_pkey on users_auth a  (cost=0.29..8.30 rows=1 width=44)
              Index Cond: (id = p.user_id)
```
Стоимость значительно упала - с 300 до 24. Причем, в EXPLAIN мы не видим использование GIN индекса idx_users_profile_last_name_trgm. Видимо, оптимизатор решил, что селектиновность условия в данном запросе по полю first_name больше - то есть значения в first_name более редкие, чем в last_name - и нет смысла еще искать по второму условию, применяя еще и второй GIN индекс, а следовательно увеличивая стоимость.

SELECT запросы можно было бы еще более оптимизировать, создав покрывающие индексы - но тогда это замедлит вставку данных в таблицы - а регистрация новых пользователей одна из целевых операций в сервисе.

Кроме этого, по заданию требуется создать индексы для FK полей - но в моем случае FK поле user_id таблицы users_profile является так же PK, а, значит, уже имеет созданный индекс.

## Список индексов, добавленных для оптимизации запросов

Индексы, добавленные для оптимизации запроса 6:
```
CREATE INDEX IF NOT EXISTS idx_users_profile_first_name_trgm ON identity.users_profile USING gin (first_name gin_trgm_ops);
CREATE INDEX IF NOT EXISTS idx_users_profile_last_name_trgm ON identity.users_profile USING gin (last_name gin_trgm_ops);
```

Два GIN индекса, по которым происходит в запросе поиск ILIKE.

## Просмотр планов запросов после оптимизаций

1. 
```
EXPLAIN SELECT id, password_hash, is_active FROM identity.users_auth WHERE login = 'ivan.petrov' LIMIT 1;
```
```
Limit  (cost=0.29..8.30 rows=1 width=130)
  ->  Index Scan using users_auth_login_key on users_auth  (cost=0.29..8.30 rows=1 width=130)
        Index Cond: ((login)::text = 'ivan.petrov'::text)
```

2. 
```
EXPLAIN SELECT login, password_hash, is_active FROM identity.users_auth WHERE id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890' LIMIT 1;
```
```
Limit  (cost=0.29..8.30 rows=1 width=130)
  ->  Index Scan using users_auth_pkey on users_auth  (cost=0.29..8.30 rows=1 width=130)
        Index Cond: (id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890'::uuid)
```

3. 
```
EXPLAIN SELECT EXISTS(SELECT 1 FROM identity.users_auth WHERE login = 'ivan.petrov');
```
```
Result  (cost=8.30..8.31 rows=1 width=1)
  InitPlan 1 (returns $0)
    ->  Index Only Scan using users_auth_login_key on users_auth  (cost=0.29..8.30 rows=1 width=0)
          Index Cond: (login = 'ivan.petrov'::text)
```

4. 
```
EXPLAIN SELECT user_id, phone, first_name, last_name, role FROM identity.users_profile WHERE email = 'ivan.petrov@test.com' LIMIT 1;
```
```
Limit  (cost=0.29..8.30 rows=1 width=91)
  ->  Index Scan using users_profile_email_key on users_profile  (cost=0.29..8.30 rows=1 width=91)
        Index Cond: ((email)::text = 'ivan.petrov@test.com'::text)
```

5. 
```
EXPLAIN SELECT email, phone, first_name, last_name, role FROM identity.users_profile WHERE user_id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890' LIMIT 1;
```
```
Limit  (cost=0.29..8.30 rows=1 width=91)
  ->  Index Scan using users_profile_pkey on users_profile  (cost=0.29..8.30 rows=1 width=91)
        Index Cond: (user_id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890'::uuid)
```

6. 
```
EXPLAIN SELECT a.id, a.login, p.email, p.phone, p.first_name, p.last_name, p.role FROM identity.users_profile p JOIN identity.users_auth a ON p.user_id = a.id WHERE p.first_name ILIKE '%Ivan%' AND p.last_name ILIKE '%Petrov%' LIMIT 100;
```
```
Limit  (cost=12.29..24.32 rows=1 width=103)
  ->  Nested Loop  (cost=12.29..24.32 rows=1 width=103)
        ->  Bitmap Heap Scan on users_profile p  (cost=12.00..16.02 rows=1 width=75)
              Recheck Cond: ((first_name)::text ~~* '%Ivan%'::text)
              Filter: ((last_name)::text ~~* '%Petrov%'::text)
              ->  Bitmap Index Scan on idx_users_profile_first_name_trgm  (cost=0.00..12.00 rows=1 width=0)
                    Index Cond: ((first_name)::text ~~* '%Ivan%'::text)
        ->  Index Scan using users_auth_pkey on users_auth a  (cost=0.29..8.30 rows=1 width=44)
              Index Cond: (id = p.user_id)
```

7. 
```
EXPLAIN SELECT EXISTS(SELECT 1 FROM identity.users_profile WHERE (email = 'ivan.petrov@test.com' OR phone = '+79991234567'));
```
```
Result  (cost=12.25..12.26 rows=1 width=1)
  InitPlan 1 (returns $0)
    ->  Bitmap Heap Scan on users_profile  (cost=8.59..15.91 rows=2 width=0)
          Recheck Cond: (((email)::text = 'ivan.petrov@test.com'::text) OR ((phone)::text = '+79991234567'::text))
          ->  BitmapOr  (cost=8.59..8.59 rows=2 width=0)
                ->  Bitmap Index Scan on users_profile_email_key  (cost=0.00..4.29 rows=1 width=0)
                      Index Cond: ((email)::text = 'ivan.petrov@test.com'::text)
                ->  Bitmap Index Scan on users_profile_phone_key  (cost=0.00..4.29 rows=1 width=0)
                      Index Cond: ((phone)::text = '+79991234567'::text)
```

8. 
```
EXPLAIN INSERT INTO identity.users_auth (login, password_hash) VALUES ('ivan.petrov', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4.G.2f3f3f3f3f3f');
```
```
Insert on users_auth  (cost=0.00..0.01 rows=0 width=0)
  ->  Result  (cost=0.00..0.01 rows=1 width=1073)
```

9. 
```
EXPLAIN INSERT INTO identity.users_profile (user_id, email, phone, first_name, last_name) VALUES ('a1b2c3d4-e5f6-7890-abcd-ef1234567890', 'ivan.petrov@test.com', '+79991234567', 'Ivan', 'Petrov');
```
```
Insert on users_profile  (cost=0.00..0.01 rows=0 width=0)
  ->  Result  (cost=0.00..0.01 rows=1 width=846)
```

## Рассуждения о результатах оптимизаций

Ожидаемо, никакие планы запросов не поменялись - кроме плана запроса 6. Стоимость запроса 6 значительно снизилась благодаря созданию двух GIN индексов для ILIKE фильтрации (полнотекстового поиска) по VARCHAR полям. В зависимости от того, по какому полю будет выше селективность, по тому пою оптимизатор PostgreSQL будет использовать GIN индекс.