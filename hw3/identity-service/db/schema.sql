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