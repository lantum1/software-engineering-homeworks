# Запросы к PostgreSQL БД

## Запросы, подлежащие оптимизации

Список запросов из [queries.sql](queries.sql) файла, на план запросов которых будем смотреть и которые будем отпимизировать:

```
INSERT INTO notification.templates (key, text) VALUES (
    'auth_credentials',
    'Ваши данные для входа:\nЛогин: {{login}}\nПароль: {{password}}'
);

SELECT id, text FROM notification.templates WHERE key = 'auth_credentials';

INSERT INTO notification.notifications (
    kafka_msg_id, user_id, type, external_system_id,
    template_id, payload, expires
) VALUES (
    'kafka-offset-011',
    '10000000-1000-1000-1000-100000000011',
    'TELEGRAM',
    '987654331',
    '11111111-1111-1111-1111-111111111111',
    '{"login": "user11@example.com", "password": "NewPass789"}',
    CURRENT_TIMESTAMP + INTERVAL '1 hour'
);

INSERT INTO notification.notifications_status (notification_id, status) VALUES (
    '55555556-5555-5555-5555-555555555556',
    'CREATED'
);

INSERT INTO notification.notifications_status (notification_id, status) VALUES (
    'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa',
    'SENT'
);

INSERT INTO notification.notifications_status (notification_id, status, description) VALUES (
    'dddddddd-dddd-dddd-dddd-dddddddddddd',
    'NOT_SENT',
    'Telegram API error 500: Internal Server Error'
);

SELECT n.id, n.type, n.external_system_id, t.text as template_text, n.payload
FROM notification.notifications n
INNER JOIN notification.notifications_status ns ON n.id = ns.notification_id
INNER JOIN notification.templates t ON n.template_id = t.id
WHERE n.expires > CURRENT_TIMESTAMP
AND ns.status = 'CREATED'
AND ns.created = (
    SELECT MAX(ns2.created) 
    FROM notification.notifications_status ns2
    WHERE ns2.notification_id = n.id
)
ORDER BY n.created
LIMIT 50;
```

## Подготовка БД перед просмотром статистики

Перед просмотром статистики выполним подготовку БД - добавим в таблицы тестовые данные.\
Перед анализом через EXPLAIN я запустил скрипт, выполнивший вставку ~20 строк в таблицу templates и ~10000 строк в notifications и notifications_status.\
Кроме этого, я вставил данные из [data.sql](data.sql) - так как сгенерированные данные являются около случайным наборот данных, то я решил дополнительно вставить заранее определнный набор данных и составить запросы на данных из этого набора.

Итоговое кол-во строк в таблице templates:
```
templates	25
```

Итоговое кол-во строк в таблице notifications:
```
notifications	10010
```

Итоговое кол-во строк в таблице notifications_status:
```
notifications_status	13999
```

## Просмотр планов запросов до оптимизаций

1. 
```
EXPLAIN INSERT INTO notification.templates (key, text) VALUES (
    'auth_credentials',
    'Ваши данные для входа:\nЛогин: {{login}}\nПароль: {{password}}'
);
```
```
Insert on templates  (cost=0.00..0.01 rows=0 width=0)
  ->  Result  (cost=0.00..0.01 rows=1 width=282)
```

2. 
```
EXPLAIN SELECT id, text FROM notification.templates WHERE key = 'auth_credentials';
```
```
Seq Scan on templates  (cost=0.00..1.06 rows=1 width=48)
  Filter: ((key)::text = 'auth_credentials'::text)
```

3. 
```
EXPLAIN INSERT INTO notification.notifications (
    kafka_msg_id, user_id, type, external_system_id,
    template_id, payload, expires
) VALUES (
    'kafka-offset-011',
    '10000000-1000-1000-1000-100000000011',
    'TELEGRAM',
    '987654331',
    '11111111-1111-1111-1111-111111111111',
    '{"login": "user11@example.com", "password": "NewPass789"}',
    CURRENT_TIMESTAMP + INTERVAL '1 hour'
);
```
```
Insert on notifications  (cost=0.00..0.02 rows=0 width=0)
  ->  Result  (cost=0.00..0.02 rows=1 width=1254)
```

4. 
```
EXPLAIN INSERT INTO notification.notifications_status (notification_id, status) VALUES (
    '55555556-5555-5555-5555-555555555556',
    'CREATED'
);
```
```
Insert on notifications_status  (cost=0.00..0.01 rows=0 width=0)
  ->  Result  (cost=0.00..0.01 rows=1 width=560)
```

5. 
```
EXPLAIN INSERT INTO notification.notifications_status (notification_id, status) VALUES (
    'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa',
    'SENT'
);
```
```
Insert on notifications_status  (cost=0.00..0.01 rows=0 width=0)
  ->  Result  (cost=0.00..0.01 rows=1 width=560)
```

6. 
```
EXPLAIN INSERT INTO notification.notifications_status (notification_id, status, description) VALUES (
    'dddddddd-dddd-dddd-dddd-dddddddddddd',
    'NOT_SENT',
    'Telegram API error 500: Internal Server Error'
);
```
```
Insert on notifications_status  (cost=0.00..0.01 rows=0 width=0)
  ->  Result  (cost=0.00..0.01 rows=1 width=560)
```

7.
```
EXPLAIN SELECT n.id, n.type, n.user_id, n.external_system_id, t.text as template_text, n.payload
FROM notification.notifications n
INNER JOIN notification.notifications_status ns ON n.id = ns.notification_id
INNER JOIN notification.templates t ON n.template_id = t.id
WHERE n.expires > CURRENT_TIMESTAMP
AND ns.status = 'CREATED'
AND ns.created = (
    SELECT MAX(ns2.created) 
    FROM notification.notifications_status ns2
    WHERE ns2.notification_id = n.id
)
ORDER BY n.created
LIMIT 50;
```
```
Limit  (cost=213339.35..213339.35 rows=1 width=120)
  ->  Sort  (cost=213339.35..213339.35 rows=1 width=120)
        Sort Key: n.created
        ->  Nested Loop  (cost=649.68..213339.34 rows=1 width=120)
              Join Filter: (n.template_id = t.id)
              ->  Hash Join  (cost=649.68..213338.23 rows=1 width=104)
                    Hash Cond: ((ns.notification_id = n.id) AND (ns.created = (SubPlan 1)))
                    ->  Seq Scan on notifications_status ns  (cost=0.00..423.99 rows=10010 width=24)
                          Filter: (status = 'CREATED'::notification.notification_status)
                    ->  Hash  (cost=614.15..614.15 rows=2369 width=104)
                          ->  Seq Scan on notifications n  (cost=0.00..614.15 rows=2369 width=104)
                                Filter: (expires > CURRENT_TIMESTAMP)
                          SubPlan 1
                            ->  Aggregate  (cost=423.99..424.00 rows=1 width=8)
                                  ->  Seq Scan on notifications_status ns2  (cost=0.00..423.99 rows=1 width=8)
                                        Filter: (notification_id = n.id)
              ->  Seq Scan on templates t  (cost=0.00..1.05 rows=5 width=48)
JIT:
  Functions: 30
  Options: Inlining false, Optimization false, Expressions true, Deforming true
```

## Рассуждения об оптимизациях

Большинство запросов, представленных для оптимизации, являются INSERT запросами.
Среди всех запросов есть два SELECT запроса - **второй** и **седьмой**.

**Второй запрос** - для получения данных шаблона из таблицы templates. Хоть и в плане запроса написано, что он использует Seq Scan, его оптимизировать нет смысла. PostgreSQL по умолчанию создает индексы на все UNIQUE поля. Поле key, фильтрация по которому происходит в запросе, имеет констрейнт UNIQUE, следовательно, по этому полю уже есть индекс. На данный момент, PostgreSQL не использует его, так как данных в таблице мало (25 строк).

**Седьмой запрос** - для получения всех необходимых данных для отправки уведомления в коммуникационную систему. Его стоит оптимизировать, добавив индексы, позволяющие проводить фильтрацию и JOIN'ы более эффективно.

Добавим следующий набор индексов:
```
CREATE INDEX IF NOT EXISTS idx_notifications_expires_created ON notification.notifications(expires, created DESC);

CREATE INDEX IF NOT EXISTS idx_notifications_status_notification_id_created ON notification.notifications_status(notification_id, created DESC);

CREATE INDEX IF NOT EXISTS idx_notifications_status_notification_id_created_only 
ON notification.notifications_status(notification_id, created DESC)
WHERE status = 'CREATED';

CREATE INDEX IF NOT EXISTS idx_templates_id_with_text ON notification.templates(id) INCLUDE (text);
```

Первый индекс должен убрать сортировку и Seq Scan таблицы notifications.

Второй индекс должен сделать выполнение подзапроса SELECT MAX(created) за O(log N) вместо Seq Scan.

Третий индекс должен ускорить фильтрацию по CREATED статусам. Кроме этого, это частичный индекс, который содержит только записи со статусом CREATED.

Четвертый индекс на данный момент может не иметь смысла, потому что в templates таблице мало записей. Но, теоретически, если и когда таблица разрастется, то данный индекс будет иметь смысл - он покрывающий и убирает необходимость обращаться к таблице templates при выполнении запроса.

Посмотрим на план 7 запроса после добавления четырех индексов выше:
```
Limit  (cost=18.31..18.32 rows=1 width=180)
  ->  Sort  (cost=18.31..18.32 rows=1 width=180)
        Sort Key: n.created
        ->  Nested Loop  (cost=12.92..18.30 rows=1 width=180)
              ->  Hash Join  (cost=8.32..9.67 rows=1 width=180)
                    Hash Cond: (t.id = n.template_id)
                    ->  Seq Scan on templates t  (cost=0.00..1.25 rows=25 width=92)
                    ->  Hash  (cost=8.30..8.30 rows=1 width=120)
                          ->  Index Scan using idx_notifications_expires_created on notifications n  (cost=0.29..8.30 rows=1 width=120)
                                Index Cond: (expires > CURRENT_TIMESTAMP)
              ->  Index Only Scan using idx_notifications_status_notification_id_created_only on notifications_status ns  (cost=4.60..8.62 rows=1 width=24)
                    Index Cond: ((notification_id = n.id) AND (created = (SubPlan 2)))
                    SubPlan 2
                      ->  Result  (cost=4.30..4.31 rows=1 width=8)
                            InitPlan 1 (returns $1)
                              ->  Limit  (cost=0.29..4.30 rows=1 width=8)
                                    ->  Index Only Scan using idx_notifications_status_notification_id_created on notifications_status ns2  (cost=0.29..4.30 rows=1 width=8)
                                          Index Cond: ((notification_id = n.id) AND (created IS NOT NULL))
```
Стоимость запроса упала в тысячи раз. Используются три из четырех созданных индексов - кроме последнего индекса idx_templates_id_with_text. Исходя из семантики таблицы templates, вряд ли когда-либо наберется много записей (1000+) в таблице с шаблонами уведомлений - вряд ли будет 1000+ различных шаблонов уведомлений. Поэтому, на данный момент данный индекс является излишним - не будем его добавлять в схему.\
А пока в таблице мало данных, то оптимизатору в любом случае будет выгоднее использовать Seq Scan вместо индекса.

Кроме этого, по заданию требуется создать индексы для FK полей - не смотря на то, что для моих запросов он не требуется.

Создадим индексы для FK:
```
CREATE INDEX IF NOT EXISTS idx_notifications_template_id ON notification.notifications(template_id);
```

Несмотря на то, что в схеме БД у меня две таблицы друг на друга ссылаются - всего два FK - я создал кастомный FK индекс только для одного FK - для notifications.template_id поля. Для notifications_status.notification_id смысла в отдельном индексе нет, потому что выше уже есть запросы на создание составного индекса, где notification_id идет первым в запросе, а, следовательно, такой индекс может использоваться для фильтрации просто по одному полю notification_id notifications_status таблицы.

## Список индексов, добавленных для оптимизации запросов

```
CREATE INDEX IF NOT EXISTS idx_notifications_expires_created ON notification.notifications(expires, created DESC);

CREATE INDEX IF NOT EXISTS idx_notifications_status_notification_id_created ON notification.notifications_status(notification_id, created DESC);

CREATE INDEX IF NOT EXISTS idx_notifications_status_notification_id_created_only 
ON notification.notifications_status(notification_id, created DESC)
WHERE status = 'CREATED';

CREATE INDEX IF NOT EXISTS idx_notifications_template_id ON notification.notifications(template_id);
```

## Просмотр планов запросов после оптимизаций

1. 
```
EXPLAIN INSERT INTO notification.templates (key, text) VALUES (
    'auth_credentials',
    'Ваши данные для входа:\nЛогин: {{login}}\nПароль: {{password}}'
);
```
```
Insert on templates  (cost=0.00..0.01 rows=0 width=0)
  ->  Result  (cost=0.00..0.01 rows=1 width=282)
```

2. 
```
EXPLAIN SELECT id, text FROM notification.templates WHERE key = 'auth_credentials';
```
```
Seq Scan on templates  (cost=0.00..1.31 rows=1 width=92)
  Filter: ((key)::text = 'auth_credentials'::text)
```

3. 
```
EXPLAIN INSERT INTO notification.notifications (
    kafka_msg_id, user_id, type, external_system_id,
    template_id, payload, expires
) VALUES (
    'kafka-offset-011',
    '10000000-1000-1000-1000-100000000011',
    'TELEGRAM',
    '987654331',
    '11111111-1111-1111-1111-111111111111',
    '{"login": "user11@example.com", "password": "NewPass789"}',
    CURRENT_TIMESTAMP + INTERVAL '1 hour'
);
```
```
Insert on notifications  (cost=0.00..0.02 rows=0 width=0)
  ->  Result  (cost=0.00..0.02 rows=1 width=1254)
```

4. 
```
EXPLAIN INSERT INTO notification.notifications_status (notification_id, status) VALUES (
    '55555556-5555-5555-5555-555555555556',
    'CREATED'
);
```
```
Insert on notifications_status  (cost=0.00..0.01 rows=0 width=0)
  ->  Result  (cost=0.00..0.01 rows=1 width=560)
```

5. 
```
EXPLAIN INSERT INTO notification.notifications_status (notification_id, status) VALUES (
    'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa',
    'SENT'
);
```
```
Insert on notifications_status  (cost=0.00..0.01 rows=0 width=0)
  ->  Result  (cost=0.00..0.01 rows=1 width=560)
```

6. 
```
EXPLAIN INSERT INTO notification.notifications_status (notification_id, status, description) VALUES (
    'dddddddd-dddd-dddd-dddd-dddddddddddd',
    'NOT_SENT',
    'Telegram API error 500: Internal Server Error'
);
```
```
Insert on notifications_status  (cost=0.00..0.01 rows=0 width=0)
  ->  Result  (cost=0.00..0.01 rows=1 width=560)
```

7.
```
EXPLAIN SELECT n.id, n.type, n.user_id, n.external_system_id, t.text as template_text, n.payload
FROM notification.notifications n
INNER JOIN notification.notifications_status ns ON n.id = ns.notification_id
INNER JOIN notification.templates t ON n.template_id = t.id
WHERE n.expires > CURRENT_TIMESTAMP
AND ns.status = 'CREATED'
AND ns.created = (
    SELECT MAX(ns2.created) 
    FROM notification.notifications_status ns2
    WHERE ns2.notification_id = n.id
)
ORDER BY n.created
LIMIT 50;
```
```
Limit  (cost=18.31..18.32 rows=1 width=180)
  ->  Sort  (cost=18.31..18.32 rows=1 width=180)
        Sort Key: n.created
        ->  Nested Loop  (cost=12.92..18.30 rows=1 width=180)
              ->  Hash Join  (cost=8.32..9.67 rows=1 width=180)
                    Hash Cond: (t.id = n.template_id)
                    ->  Seq Scan on templates t  (cost=0.00..1.25 rows=25 width=92)
                    ->  Hash  (cost=8.30..8.30 rows=1 width=120)
                          ->  Index Scan using idx_notifications_expires_created on notifications n  (cost=0.29..8.30 rows=1 width=120)
                                Index Cond: (expires > CURRENT_TIMESTAMP)
              ->  Index Only Scan using idx_notifications_status_notification_id_created_only on notifications_status ns  (cost=4.60..8.62 rows=1 width=24)
                    Index Cond: ((notification_id = n.id) AND (created = (SubPlan 2)))
                    SubPlan 2
                      ->  Result  (cost=4.30..4.31 rows=1 width=8)
                            InitPlan 1 (returns $1)
                              ->  Limit  (cost=0.29..4.30 rows=1 width=8)
                                    ->  Index Only Scan using idx_notifications_status_notification_id_created on notifications_status ns2  (cost=0.29..4.30 rows=1 width=8)
                                          Index Cond: ((notification_id = n.id) AND (created IS NOT NULL))
```

## Рассуждения о результатах оптимизаций

Никакая стоимость запросов, кроме запроса 7, не изменилась после добавления индексов. Стоимость 7 запроса упала значительно благодаря 3 индексам - idx_notifications_expires_created, idx_notifications_status_notification_id_created_only и idx_notifications_status_notification_id_created. Покрывающий индекс на получение поля text из таблицы templates (idx_templates_id_with_text) в 7 запросе пока что решил не добавлять, потому что в дальней перспективе семантики таблицы данный индекс будет излишним.