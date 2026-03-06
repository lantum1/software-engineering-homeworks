workspace {
    !identifiers hierarchical
    !impliedRelationships false

    name "MAX Disk 360"
    description "Отечественная онлайн система хранения файлов MAX Disk 360"

    model {
        customer = person "Пользователь" {
            description "Пользователь системы - целевой клиент, потребитель"
        }
        
        telegram = softwareSystem "Telegram" {
            description "Внешняя платформа для доставки сообщений пользователю"
            tags "ExternalSystem"
        }
        maxDisk360 = softwareSystem "MAX Disk 360" {
            description "Отечественная онлайн система хранения файлов"

            identityDb = container "Identity Database" {
                description "Хранит пользователей и их роли"
                technology "PostgreSQL"
                tags "Database"
            }

            fileMetadataDb = container "File Metadata DB" {
                description "Хранит метаданные файлов пользователей"
                technology "MongoDB"
                tags "Database"
            }

            notificationDb = container "Notification Database" {
                description "Хранит параметры сообщений и очередь для их отправки"
                technology "PostgreSQL"
                tags "Database"
            }

            notificationService = container "Notification Service" {
                description "Отправляет сообщения пользователям через Telegram"
                technology "C++ Poco"
                -> notificationDb "Читает шаблоны сообщений и сохраняет записи Outbox" "PostgreSQL (TCP/IP)"
                -> telegram "Отправляет сообщение пользователю" "HTTPS/Telegram API"
            }

            kafka = container "Event Bus" {
                description "Брокер сообщений для асинхронного взаимодействия сервисов"
                technology "Kafka"
                tags "EventBus"
                -> notificationService "Передает событие о необходимости отправить авторизационные данные пользователя" "Kafka"
            }

            identityService = container "Identity Service" {
                description "Отвечает за аутентификацию, регистрацию, генерацию JWT токенов и управление учетными записями пользователей"
                technology "C++ Poco"
                -> identityDb "Создает и ищет пользователей" "PostgreSQL (TCP/IP)"
                -> kafka "Публикует событие о необходимости отправить авторизационные данные пользователя" "Kafka"
            }

            fileManagementService = container "File Management Service" {
                description "Управляет файлами и метаданными пользователей"
                technology "C++ Poco"
                -> fileMetadataDb "Читает, ищет и записывает метаданные о файлах и папках" "MongoDB (TCP/IP)"
            }

            gateway = container "API Gateway" {
                description "Проверяет JWT токены, роль пользователей и проксирует запросы к внутренним сервисам"
                technology "C++ Poco"
                -> identityService "Проксирует запросы регистрации, логина и поиска пользователя" "HTTP/REST"
                -> fileManagementService "Проксирует запросы по работе с файлами и папками" "HTTP/REST"
            }
        }

        customer -> maxDisk360 "Регистрируется, аутентифицируется и управляет своими файлами"
        customer -> maxDisk360.gateway "Использует методы REST API системы" "HTTP/REST"

        maxDisk360 -> telegram "Отправляет логин и пароль пользователю при регистрации"
    }

    views {
        systemContext maxDisk360 "C1" {
            include *
            autolayout lr
        }
        container maxDisk360 "C2" {
            include *
            autolayout lr
        }
        dynamic maxDisk360 "CreatingNewFileByAuthorizedCustomer" {
            description "Сценарий: создание нового файла авторизованным пользователем"
            autolayout lr

            customer -> maxDisk360.gateway "POST /createFile (с JWT токеном)"
            maxDisk360.gateway -> maxDisk360.fileManagementService "Проксирование запроса"

            maxDisk360.fileManagementService -> maxDisk360.fileMetadataDb "Сохранение метаданных о новом файле"

            maxDisk360.fileManagementService -> maxDisk360.gateway "HTTP 201 ответ"
            maxDisk360.gateway -> customer "Ответ об успешной загрузке"
        }
        styles {
            element "ExternalSystem" {
                background #dddddd
                color #000000
            }

            element "Database" {
                shape Cylinder
            }

            element "EventBus" {
                background "#f5da81"
                color "#000000"
                shape Pipe
            }
        }
        theme default
    }
}