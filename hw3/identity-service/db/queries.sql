-- Вставка новой записи в таблицу с данными аутентификации пользователя
INSERT INTO identity.users_auth (login, password_hash) VALUES ('ivan.petrov', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4.G.2f3f3f3f3f3f');

-- Вставка новой записи в таблицу с данными профиля пользователя
INSERT INTO identity.users_profile (user_id, email, phone, first_name, last_name) VALUES ('a1b2c3d4-e5f6-7890-abcd-ef1234567890', 'ivan.petrov@test.com', '+79991234567', 'Ivan', 'Petrov');

-- Получение ID пользователя, хеша пароля и флага активного пользователя по логину
SELECT id, password_hash, is_active FROM identity.users_auth WHERE login = 'ivan.petrov' LIMIT 1;

-- Получение логина пользователя, хеша пароля и флага активного пользователя по ID пользователя
SELECT login, password_hash, is_active FROM identity.users_auth WHERE id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890' LIMIT 1;

-- Проверка, существует ли пользователь с данным логином
SELECT EXISTS(SELECT 1 FROM identity.users_auth WHERE login = 'ivan.petrov');

-- Получение ID пользователя, номера телефона, имени, фамилии и роли по email
SELECT user_id, phone, first_name, last_name, role FROM identity.users_profile WHERE email = 'ivan.petrov@test.com' LIMIT 1;

-- Получение email пользователя, номера телефона, имени, фамилии и роли по ID пользователя
SELECT email, phone, first_name, last_name, role FROM identity.users_profile WHERE user_id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890' LIMIT 1;

-- Получение ID пользователя, логина, email, номера телефона, имени, фамилии и роли по маске фамилии и имени
SELECT a.id, a.login, p.email, p.phone, p.first_name, p.last_name, p.role FROM identity.users_profile p JOIN identity.users_auth a ON p.user_id = a.id WHERE p.first_name ILIKE '%Ivan%' AND p.last_name ILIKE '%Petrov%' LIMIT 100;

-- Проверка, существует ли пользователь с данным email или номером телефона
SELECT EXISTS(SELECT 1 FROM identity.users_profile WHERE (email = 'ivan.petrov@test.com' OR phone = '+79991234567'));