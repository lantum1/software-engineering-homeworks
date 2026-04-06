-- Вставка нового шаблона
INSERT INTO notification.templates (key, text) VALUES (
    'auth_credentials',
    'Ваши данные для входа:\nЛогин: {{login}}\nПароль: {{password}}'
);

-- Получение шаблона по имени
SELECT id, text FROM notification.templates WHERE key = 'auth_credentials';

-- Вставка нового запроса на отправку уведомления
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

-- Вставка новых статусов отправки уведомления
-- CREATED (начальный статус)
INSERT INTO notification.notifications_status (notification_id, status) VALUES (
    '55555556-5555-5555-5555-555555555556',
    'CREATED'
);

-- SENT (успешная отправка уведомления)
INSERT INTO notification.notifications_status (notification_id, status) VALUES (
    'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa',
    'SENT'
);

-- NOT_SENT (ошибка отправки уведомления)
INSERT INTO notification.notifications_status (notification_id, status, description) VALUES (
    'dddddddd-dddd-dddd-dddd-dddddddddddd',
    'NOT_SENT',
    'Telegram API error 500: Internal Server Error'
);

-- Выбор уведомлений для отправки и необходимых для отправки данных (по шедулеру)
SELECT n.id, n.type, n.user_id, n.external_system_id, t.text as template_text, n.payload
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