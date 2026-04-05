INSERT INTO identity.users_auth (id, login, password_hash, is_active, last_login_at) VALUES
('a1b2c3d4-e5f6-7890-abcd-ef1234567890', 'ivan.petrov', '$2b$12$KIXxPZvJ8qYqYqYqYqYqYqYqYqYqYqYqYqYqYqYqYqYqYqYqYqYq', TRUE, CURRENT_TIMESTAMP),
('b2c3d4e5-f6a7-8901-bcde-f12345678901', 'maria.sokolova', '$2b$12$ABCDEFGHIJKLMNOPQRSTUVWXyz1234567890abcdefghijklmno', TRUE, CURRENT_TIMESTAMP),
('c3d4e5f6-a7b8-9012-cdef-123456789012', 'alexey.ivanov', '$2b$12$QRSTUVWXYZ123456789ABCDEFGHIJKLMNOPQRSTUVWXyz1234', FALSE, NULL),
('d4e5f6a7-b8c9-0123-def1-234567890123', 'elena.kozlova', '$2b$12$567890ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcde', TRUE, CURRENT_TIMESTAMP),
('e5f6a7b8-c9d0-1234-ef12-345678901234', 'dmitry.novikov', '$2b$12$fghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQR', TRUE, NULL),
('f6a7b8c9-d0e1-2345-f123-456789012345', 'olga.smirnova', '$2b$12$STUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzABC', FALSE, NULL),
('a7b8c9d0-e1f2-3456-1234-567890123456', 'sergey.volkov', '$2b$12$DEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnop', TRUE, CURRENT_TIMESTAMP),
('b8c9d0e1-f2a3-4567-2345-678901234567', 'anna.morozova', '$2b$12$QRSTUVWXYZ1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZa', TRUE, CURRENT_TIMESTAMP),
('c9d0e1f2-a3b4-5678-3456-789012345678', 'pavel.lebedev', '$2b$12$bcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLM', FALSE, NULL),
('d0e1f2a3-b4c5-6789-4567-890123456789', 'natasha.pavlova', '$2b$12$NOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz', TRUE, CURRENT_TIMESTAMP),
('e1f2a3b4-c5d6-7890-5678-901234567890', 'admin', '$2b$12$ADMINHASH1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZab', TRUE, CURRENT_TIMESTAMP);

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