#pragma once

#include "INotificationPublisher.h"
#include <iostream>

using namespace std;

namespace maxdisk::identity::notification
{

    /**
     * Заглушка для INotificationPublisher.
     *
     * Логирует событие в консоль. Далее в следующих ЛР будет заменена реальной отправкой в Kafka события об отправке сообщения о регистрации
     */
    class StubNotificationPublisher : public INotificationPublisher
    {
    public:
        void publishCredentialsEvent(const string &userId, const string &email, const string &login, const string &password) override
        {
            cerr << "Публикуем событие об отправке сообщения о регистрации в Кафка:\n"
                 << "  userId: " << userId << "\n"
                 << "  email: " << email << "\n"
                 << "  login: " << login << "\n"
                 << "  password: " << password << "\n";
        }
    };

}