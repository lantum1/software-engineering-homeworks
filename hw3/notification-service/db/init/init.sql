CREATE SCHEMA IF NOT EXISTS notification;

CREATE TABLE notification.templates (
    id          UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    key         VARCHAR(100) NOT NULL UNIQUE,
    text        TEXT NOT NULL,
    created     TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated     TIMESTAMP WITH TIME ZONE
);

CREATE TABLE notification.notifications (
    id                 UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    kafka_msg_id       VARCHAR(255) NOT NULL UNIQUE,
    user_id            UUID NOT NULL,
    type               VARCHAR(50) NOT NULL,
    external_system_id VARCHAR(255) NOT NULL,
    template_id        UUID NOT NULL REFERENCES notification.templates(id),
    payload            JSONB NOT NULL,
    expires            TIMESTAMP WITH TIME ZONE NOT NULL,
    created            TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated            TIMESTAMP WITH TIME ZONE
);

CREATE TYPE notification.notification_status AS ENUM (
    'CREATED',
    'SENT',
    'NOT_SENT'
);

CREATE TABLE notification.notifications_status (
    id              UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    notification_id UUID NOT NULL REFERENCES notification.notifications(id) ON DELETE CASCADE,
    status          notification.notification_status NOT NULL,
    description     VARCHAR(255),
    created         TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_notifications_expires_created ON notification.notifications(expires, created DESC);

CREATE INDEX IF NOT EXISTS idx_notifications_status_notification_id_created ON notification.notifications_status(notification_id, created DESC);

CREATE INDEX IF NOT EXISTS idx_notifications_status_notification_id_created_only 
ON notification.notifications_status(notification_id, created DESC)
WHERE status = 'CREATED';

CREATE INDEX IF NOT EXISTS idx_templates_id_with_text ON notification.templates(id) INCLUDE (text);

CREATE INDEX IF NOT EXISTS idx_notifications_template_id ON notification.notifications(template_id);

-- ============================================
-- Вставка тестовых данных в таблицу notification.templates - шаблоны уведомлений
-- ============================================
INSERT INTO notification.templates (id, key, text, created, updated) VALUES
('11111111-1111-1111-1111-111111111111', 
 'auth_credentials', 
 'Ваши данные для входа:\nЛогин: {{login}}\nПароль: {{password}}', 
 NOW() - INTERVAL '30 days', 
 NOW() - INTERVAL '5 days'),
('22222222-2222-2222-2222-222222222222', 
 'password_reset', 
 'Запрос на сброс пароля.\nКод подтверждения: {{code}}\nДействует 10 минут.', 
 NOW() - INTERVAL '25 days', 
 NULL),
('33333333-3333-3333-3333-333333333333', 
 'welcome_message', 
 'Добро пожаловать в MAX Disk 360, {{username}}!\nВам доступно {{storage_gb}} ГБ.', 
 NOW() - INTERVAL '20 days', 
 NOW() - INTERVAL '2 days'),
('44444444-4444-4444-4444-444444444444', 
 'storage_warning', 
 'Внимание! Заполнено {{used_percent}}% хранилища.\nОсталось {{remaining_gb}} ГБ.', 
 NOW() - INTERVAL '15 days', 
 NULL),
('55555555-5555-5555-5555-555555555555', 
 'file_shared', 
 'Пользователь {{sharer_name}} поделился файлом: {{file_name}}', 
 NOW() - INTERVAL '10 days', 
 NOW() - INTERVAL '1 day');

-- ============================================
-- Вставка тестовых данных в таблицу notification.notifications - INBOX/OUTBOX для отправки уведомлений
-- ============================================
INSERT INTO notification.notifications (
    id, kafka_msg_id, user_id, type, external_system_id, 
    template_id, payload, expires, created, updated
) VALUES
('aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa', 
 'kafka-offset-001', 
 '10000000-1000-1000-1000-100000000001', 
 'TELEGRAM', 
 '987654321', 
 '11111111-1111-1111-1111-111111111111', 
 '{"login": "user1@example.com", "password": "TempPass123"}', 
 NOW() + INTERVAL '1 hour', 
 NOW() - INTERVAL '2 hours', 
 NOW() - INTERVAL '1 hour'),
('bbbbbbbb-bbbb-bbbb-bbbb-bbbbbbbbbbbb', 
 'kafka-offset-002', 
 '10000000-1000-1000-1000-100000000002', 
 'TELEGRAM', 
 '987654322', 
 '22222222-2222-2222-2222-222222222222', 
 '{"code": "458921"}', 
 NOW() + INTERVAL '1 hour', 
 NOW() - INTERVAL '3 hours', 
 NOW() - INTERVAL '2 hours'),
('cccccccc-cccc-cccc-cccc-cccccccccccc', 
 'kafka-offset-003', 
 '10000000-1000-1000-1000-100000000003', 
 'TELEGRAM', 
 '987654323', 
 '33333333-3333-3333-3333-333333333333', 
 '{"username": "ivanov", "storage_gb": 50}', 
 NOW() - INTERVAL '1 hour', 
 NOW() - INTERVAL '3 hours', 
 NOW() - INTERVAL '2 hours'),
('dddddddd-dddd-dddd-dddd-dddddddddddd', 
 'kafka-offset-004', 
 '10000000-1000-1000-1000-100000000004', 
 'TELEGRAM', 
 '987654324', 
 '44444444-4444-4444-4444-444444444444', 
 '{"used_percent": 95, "remaining_gb": 2}', 
 NOW() + INTERVAL '1 hour', 
 NOW() - INTERVAL '4 hours', 
 NOW() - INTERVAL '3 hours'),
('eeeeeeee-eeee-eeee-eeee-eeeeeeeeeeee', 
 'kafka-offset-005', 
 '10000000-1000-1000-1000-100000000005', 
 'TELEGRAM', 
 '987654325', 
 '55555555-5555-5555-5555-555555555555', 
 '{"sharer_name": "Петров А.", "file_name": "report.pdf"}', 
 NOW() + INTERVAL '1 hour', 
 NOW() - INTERVAL '5 hours', 
 NOW() - INTERVAL '4 hours'),
('ffffffff-ffff-ffff-ffff-ffffffffffff', 
 'kafka-offset-006', 
 '10000000-1000-1000-1000-100000000006', 
 'TELEGRAM', 
 '987654326', 
 '11111111-1111-1111-1111-111111111111', 
 '{"login": "user6@example.com", "password": "SecurePass456"}', 
 NOW() + INTERVAL '1 hour', 
 NOW() - INTERVAL '6 hours', 
 NOW() - INTERVAL '5 hours'),
('11111112-1111-1111-1111-111111111112', 
 'kafka-offset-007', 
 '10000000-1000-1000-1000-100000000007', 
 'TELEGRAM', 
 '987654327', 
 '22222222-2222-2222-2222-222222222222', 
 '{"code": "789456"}', 
 NOW() + INTERVAL '1 hour', 
 NOW() - INTERVAL '7 hours', 
 NOW() - INTERVAL '6 hours'),
('22222223-2222-2222-2222-222222222223', 
 'kafka-offset-008', 
 '10000000-1000-1000-1000-100000000008', 
 'TELEGRAM', 
 '987654328', 
 '33333333-3333-3333-3333-333333333333', 
 '{"username": "smirnova", "storage_gb": 100}', 
 NOW() + INTERVAL '1 hour', 
 NOW() - INTERVAL '8 hours', 
 NOW() - INTERVAL '7 hours'),
('33333334-3333-3333-3333-333333333334', 
 'kafka-offset-009', 
 '10000000-1000-1000-1000-100000000009', 
 'TELEGRAM', 
 '987654329', 
 '44444444-4444-4444-4444-444444444444', 
 '{"used_percent": 80, "remaining_gb": 10}', 
 NOW() + INTERVAL '1 hour', 
 NOW() - INTERVAL '9 hours', 
 NOW() - INTERVAL '8 hours'),
('44444445-4444-4444-4444-444444444445', 
 'kafka-offset-010', 
 '10000000-1000-1000-1000-100000000010', 
 'TELEGRAM', 
 '987654330', 
 '55555555-5555-5555-5555-555555555555', 
 '{"sharer_name": "Сидоров В.", "file_name": "contract.docx"}', 
 NOW() + INTERVAL '1 hour', 
 NOW() - INTERVAL '10 hours', 
 NOW() - INTERVAL '9 hours');

-- ============================================
-- Вставка тестовых данных в таблицу notification.notifications_status - статусы отправки уведомлений
-- ============================================
INSERT INTO notification.notifications_status (id, notification_id, status, description, created) VALUES
(gen_random_uuid(), 'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa', 'CREATED', NULL, NOW() - INTERVAL '2 hours'),
(gen_random_uuid(), 'aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa', 'SENT', NULL, NOW() - INTERVAL '1 hour 55 minutes');
INSERT INTO notification.notifications_status (id, notification_id, status, description, created) VALUES
(gen_random_uuid(), 'bbbbbbbb-bbbb-bbbb-bbbb-bbbbbbbbbbbb', 'CREATED', NULL, NOW() - INTERVAL '3 hours'),
(gen_random_uuid(), 'bbbbbbbb-bbbb-bbbb-bbbb-bbbbbbbbbbbb', 'SENT', NULL, NOW() - INTERVAL '2 hours 50 minutes');
INSERT INTO notification.notifications_status (id, notification_id, status, description, created) VALUES
(gen_random_uuid(), 'cccccccc-cccc-cccc-cccc-cccccccccccc', 'CREATED', NULL, NOW() - INTERVAL '3 hours'),
(gen_random_uuid(), 'cccccccc-cccc-cccc-cccc-cccccccccccc', 'NOT_SENT', 'Notification lifetime exceeded', NOW() - INTERVAL '1 hour');
INSERT INTO notification.notifications_status (id, notification_id, status, description, created) VALUES
(gen_random_uuid(), 'dddddddd-dddd-dddd-dddd-dddddddddddd', 'CREATED', NULL, NOW() - INTERVAL '4 hours'),
(gen_random_uuid(), 'dddddddd-dddd-dddd-dddd-dddddddddddd', 'NOT_SENT', 'Telegram error 500: Internal Server Error', NOW() - INTERVAL '3 hours 30 minutes');
INSERT INTO notification.notifications_status (id, notification_id, status, description, created) VALUES
(gen_random_uuid(), 'eeeeeeee-eeee-eeee-eeee-eeeeeeeeeeee', 'CREATED', NULL, NOW() - INTERVAL '5 hours'),
(gen_random_uuid(), 'eeeeeeee-eeee-eeee-eeee-eeeeeeeeeeee', 'SENT', NULL, NOW() - INTERVAL '4 hours 55 minutes');
INSERT INTO notification.notifications_status (id, notification_id, status, description, created) VALUES
(gen_random_uuid(), 'ffffffff-ffff-ffff-ffff-ffffffffffff', 'CREATED', NULL, NOW() - INTERVAL '6 hours'),
(gen_random_uuid(), 'ffffffff-ffff-ffff-ffff-ffffffffffff', 'SENT', NULL, NOW() - INTERVAL '5 hours 50 minutes');
INSERT INTO notification.notifications_status (id, notification_id, status, description, created) VALUES
(gen_random_uuid(), '11111112-1111-1111-1111-111111111112', 'CREATED', NULL, NOW() - INTERVAL '7 hours'),
(gen_random_uuid(), '11111112-1111-1111-1111-111111111112', 'NOT_SENT', 'Telegram error 403: Forbidden', NOW() - INTERVAL '6 hours 30 minutes');
INSERT INTO notification.notifications_status (id, notification_id, status, description, created) VALUES
(gen_random_uuid(), '22222223-2222-2222-2222-222222222223', 'CREATED', NULL, NOW() - INTERVAL '8 hours'),
(gen_random_uuid(), '22222223-2222-2222-2222-222222222223', 'SENT', NULL, NOW() - INTERVAL '7 hours 55 minutes');
INSERT INTO notification.notifications_status (id, notification_id, status, description, created) VALUES
(gen_random_uuid(), '33333334-3333-3333-3333-333333333334', 'CREATED', NULL, NOW() - INTERVAL '9 hours'),
(gen_random_uuid(), '33333334-3333-3333-3333-333333333334', 'NOT_SENT', 'Connection timeout', NOW() - INTERVAL '8 hours 30 minutes');
INSERT INTO notification.notifications_status (id, notification_id, status, description, created) VALUES
(gen_random_uuid(), '44444445-4444-4444-4444-444444444445', 'CREATED', NULL, NOW() - INTERVAL '10 hours'),
(gen_random_uuid(), '44444445-4444-4444-4444-444444444445', 'SENT', NULL, NOW() - INTERVAL '9 hours 55 minutes');