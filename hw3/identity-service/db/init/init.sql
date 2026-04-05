CREATE SCHEMA IF NOT EXISTS identity;

CREATE TYPE identity.user_role AS ENUM ('user', 'admin');

CREATE TABLE identity.users_auth (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    login VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    last_login_at TIMESTAMP WITH TIME ZONE,
    created TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated TIMESTAMP WITH TIME ZONE,
    CONSTRAINT check_password_hash_length CHECK (char_length(password_hash) >= 60)
);

CREATE TABLE identity.users_profile (
    user_id UUID PRIMARY KEY,
    email VARCHAR(255) UNIQUE NOT NULL,
    phone VARCHAR(20) UNIQUE NOT NULL,
    first_name VARCHAR(50) NOT NULL,
    last_name VARCHAR(50) NOT NULL,
    role identity.user_role NOT NULL DEFAULT 'user',
    created TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated TIMESTAMP WITH TIME ZONE,
    CONSTRAINT fk_users_profile_auth FOREIGN KEY (user_id) REFERENCES identity.users_auth(id) ON DELETE CASCADE,
    CONSTRAINT check_phone_format CHECK (phone ~ '^\+[1-9]\d{7,14}$'),
    CONSTRAINT check_email_format CHECK (email ~ '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$')
);

CREATE EXTENSION IF NOT EXISTS pg_trgm;
CREATE INDEX IF NOT EXISTS idx_users_profile_first_name_trgm ON identity.users_profile USING gin (first_name gin_trgm_ops);
CREATE INDEX IF NOT EXISTS idx_users_profile_last_name_trgm ON identity.users_profile USING gin (last_name gin_trgm_ops);

INSERT INTO identity.users_auth (id, login, password_hash, is_active, last_login_at) VALUES
('a1b2c3d4-e5f6-7890-abcd-ef1234567890', 'ivan.petrov', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4.G.2f3f3f3f3f3f', TRUE, CURRENT_TIMESTAMP),
('b2c3d4e5-f6a7-8901-bcde-f12345678901', 'maria.sokolova', '$2b$12$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi', TRUE, CURRENT_TIMESTAMP),
('c3d4e5f6-a7b8-9012-cdef-123456789012', 'alexey.ivanov', '$2b$12$8gHVTxZzJ9KqL5NmPqRsOuVxWz3Y4A5B6C7D8E9F0G1H2I3J4K5L6', FALSE, NULL),
('d4e5f6a7-b8c9-0123-def1-234567890123', 'elena.kozlova', '$2b$12$M7N8O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M', TRUE, CURRENT_TIMESTAMP),
('e5f6a7b8-c9d0-1234-ef12-345678901234', 'dmitry.novikov', '$2b$12$N3O4P5Q6R7S8T9U0V1W2X3Y4Z5A6B7C8D9E0F1G2H3I4J5K6L7M8N', TRUE, NULL),
('f6a7b8c9-d0e1-2345-f123-456789012345', 'olga.smirnova', '$2b$12$O9P0Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O', FALSE, NULL),
('a7b8c9d0-e1f2-3456-1234-567890123456', 'sergey.volkov', '$2b$12$P5Q6R7S8T9U0V1W2X3Y4Z5A6B7C8D9E0F1G2H3I4J5K6L7M8N9O0P', TRUE, CURRENT_TIMESTAMP),
('b8c9d0e1-f2a3-4567-2345-678901234567', 'anna.morozova', '$2b$12$Q1R2S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q', TRUE, CURRENT_TIMESTAMP),
('c9d0e1f2-a3b4-5678-3456-789012345678', 'pavel.lebedev', '$2b$12$R7S8T9U0V1W2X3Y4Z5A6B7C8D9E0F1G2H3I4J5K6L7M8N9O0P1Q2R', FALSE, NULL),
('d0e1f2a3-b4c5-6789-4567-890123456789', 'natasha.pavlova', '$2b$12$S3T4U5V6W7X8Y9Z0A1B2C3D4E5F6G7H8I9J0K1L2M3N4O5P6Q7R8S', TRUE, CURRENT_TIMESTAMP),
('e1f2a3b4-c5d6-7890-5678-901234567890', 'admin', '$2b$12$T9U0V1W2X3Y4Z5A6B7C8D9E0F1G2H3I4J5K6L7M8N9O0P1Q2R3S4T', TRUE, CURRENT_TIMESTAMP);

INSERT INTO identity.users_profile (user_id, email, phone, first_name, last_name, role) VALUES
('a1b2c3d4-e5f6-7890-abcd-ef1234567890', 'ivan.petrov@mail.ru', '+79991234567', 'Ivan', 'Petrov', 'user'),
('b2c3d4e5-f6a7-8901-bcde-f12345678901', 'maria.sokolova@yandex.ru', '+79992345678', 'Maria', 'Sokolova', 'user'),
('c3d4e5f6-a7b8-9012-cdef-123456789012', 'alexey.ivanov@gmail.com', '+79993456789', 'Alexey', 'Ivanov', 'user'),
('d4e5f6a7-b8c9-0123-def1-234567890123', 'elena.kozlova@mail.ru', '+79994567890', 'Elena', 'Kozlova', 'user'),
('e5f6a7b8-c9d0-1234-ef12-345678901234', 'dmitry.novikov@outlook.com', '+79995678901', 'Dmitry', 'Novikov', 'user'),
('f6a7b8c9-d0e1-2345-f123-456789012345', 'olga.smirnova@yandex.ru', '+79996789012', 'Olga', 'Smirnova', 'user'),
('a7b8c9d0-e1f2-3456-1234-567890123456', 'sergey.volkov@mail.ru', '+79997890123', 'Sergey', 'Volkov', 'user'),
('b8c9d0e1-f2a3-4567-2345-678901234567', 'anna.morozova@gmail.com', '+79998901234', 'Anna', 'Morozova', 'user'),
('c9d0e1f2-a3b4-5678-3456-789012345678', 'pavel.lebedev@yandex.ru', '+79999012345', 'Pavel', 'Lebedev', 'user'),
('d0e1f2a3-b4c5-6789-4567-890123456789', 'natasha.pavlova@mail.ru', '+79990123456', 'Natasha', 'Pavlova', 'user'),
('e1f2a3b4-c5d6-7890-5678-901234567890', 'admin@maxdisk360.ru', '+79990000001', 'Admin', 'Adminov', 'admin');