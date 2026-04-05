INSERT INTO identity.users_auth (login, password_hash) VALUES ('ivan.petrov', '$2b$12$LQv3c1yqBWVHxkd0LHAkCOYz6TtxMQJqhN8/X4.G.2f3f3f3f3f3f');

SELECT id, password_hash, is_active FROM identity.users_auth WHERE login = 'ivan.petrov' LIMIT 1;

SELECT login, password_hash, is_active FROM identity.users_auth WHERE id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890' LIMIT 1;

SELECT EXISTS(SELECT 1 FROM identity.users_auth WHERE login = 'ivan.petrov');

SELECT user_id, phone, first_name, last_name, role FROM identity.users_profile WHERE email = 'ivan.petrov@test.com' LIMIT 1;

SELECT email, phone, first_name, last_name, role FROM identity.users_profile WHERE user_id = 'a1b2c3d4-e5f6-7890-abcd-ef1234567890' LIMIT 1;

SELECT a.id, a.login, p.email, p.phone, p.first_name, p.last_name, p.role FROM identity.users_profile p JOIN identity.users_auth a ON p.user_id = a.id WHERE p.first_name ILIKE '%Ivan%' AND p.last_name ILIKE '%Petrov%' LIMIT 100;

INSERT INTO identity.users_profile (user_id, email, phone, first_name, last_name) VALUES ('a1b2c3d4-e5f6-7890-abcd-ef1234567890', 'ivan.petrov@test.com', '+79991234567', 'Ivan', 'Petrov');

SELECT EXISTS(SELECT 1 FROM identity.users_profile WHERE (email = 'ivan.petrov@test.com' OR phone = '+79991234567'));