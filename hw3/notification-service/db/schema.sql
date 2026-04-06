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