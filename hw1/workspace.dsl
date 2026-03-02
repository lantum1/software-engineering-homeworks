workspace {    
    name "MAX Disk 360"
    description "Отечественная онлайн система хранения файлов MAX Disk 360"

    model {
        customer = person "Потребитель" {
            description "Пользователь системы - целевой клиент, потребитель"
            tags "Customer"
        }
        admin = person "Администратор" {
            description "Пользователь системы - сотрудник команды поддержки системы"
            tags "Person"
        }
        
        telegram = softwareSystem "Telegram" {
            description "Внешняя платформа для доставки сообщений Потребителям"
            tags "ExternalSystem"
        }
        maxDisk360 = softwareSystem "MAX Disk 360" {
            description "Отечественная онлайн система хранения файлов"

            gateway = container "API Gateway" {
                description "Проверяет JWT токены, роль пользователей и проксирует запросы к внутренним сервисам"
                technology "C++ Poco"
            }

            identityService = container "Identity Service" {
                description "Отвечает за аутентификацию, генерацию JWT токенов и управление учетными записями пользователей"
                technology "C++ Poco"
            }

            identityDb = container "Identity Database" {
                description "Хранит пользователей и их роли"
                technology "PostgreSQL"
                tags "Database"
            }

            fileManagementService = container "File Management Service" {
                description "Управляет файлами и метаданными Потребителей"
                technology "C++ Poco"
            }

            fileMetadataDb = container "File Metadata DB" {
                description "Хранит метаданные файлов Потребителей"
                technology "PostgreSQL"
                tags "Database"
            }

            fileStorage = container "File Storage" {
                description "Хранит бинарные данные файлов Потребителей"
                technology "MinIO (S3)"
                tags "ObjectStorage"
            }

            customer -> gateway "Использует методы REST API системы" "HTTP/REST"
            admin -> gateway "Использует методы REST API с возможностью административного доступа" "HTTP/REST"

            gateway -> identityService "Проксирует запросы регистрации и логина" "HTTP/REST"
            gateway -> fileManagementService "Проксирует защищённые запросы (после валидации JWT)" "HTTP/REST"

            identityService -> identityDb "Создает и ищет пользователей" "PostgreSQL (TCP/IP)"
            identityService -> telegram "Отправляет логин и пароль Потребителю" "HTTPS/Telegram API"

            fileManagementService -> fileMetadataDb "Читает, ищет и записывает метаданные о файлах и папках" "PostgreSQL (TCP/IP)"
            fileManagementService -> fileStorage "Сохраняет и удаляет файлы и папки" "HTTP/S3 API"
        }

        customer -> maxDisk360 "Регистрируется, аутентифицируется и управляет своими файлами"
        admin -> maxDisk360 "Регистрирует Потребителя, ищет Потребителей"

        maxDisk360 -> telegram "Отправляет логин и пароль Потребителю при регистрации"
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
        dynamic maxDisk360 "UploadingFileByAuthorizedCustomer" {
            description "Сценарий: загрузка файла авторизованным Потребителем"
            autolayout lr

            customer -> gateway "POST /upload (с JWT токеном)"
            gateway -> fileManagementService "Проксирование запроса"

            fileManagementService -> fileStorage "Сохранение бинарных данных файла"
            fileManagementService -> fileMetadataDb "Сохранение метаданных"

            fileManagementService -> gateway "HTTP 201 ответ"
            gateway -> customer "Ответ об успешной загрузке"
        }
        styles {
            element "ExternalSystem" {
                background #dddddd
                color #000000
            }

            element "Person" {
                shape Person
                background #08427b
                color #ffffff
            }

            element "Customer" {
                background #1168bd
                color #ffffff
            }

            element "Database" {
                shape Cylinder
            }

            element "ObjectStorage" {
                shape Cylinder
                background #f5da81
            }
        }
        theme default
    }
}