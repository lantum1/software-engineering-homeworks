// Создание БД, коллекций и индексов
use("file-management");

db.createCollection("folders");

db.createCollection("files");

db.folders.createIndex({ userId: 1, name: 1 }, { unique: true });

db.files.createIndex({ folderId: 1, name: 1 }, { unique: true });
db.files.createIndex({ folderId: 1, userId: 1 });

// Добавление валидации на документы коллекции - из validation.js
db.runCommand({
  collMod: "folders",
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: ["_id", "userId", "name", "metadata"],
      properties: {
        _id: {
          bsonType: "binData",
          pattern: "^[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$"
        },
        userId: {
          bsonType: "binData",
          pattern: "^[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$"
        },
        name: {
          bsonType: "string",
          minLength: 1,
          maxLength: 255
        },
        metadata: {
          bsonType: "object",
          required: ["created"],
          properties: {
            created: { bsonType: "date" },
            updated: { bsonType: "date" }
          }
        }
      }
    }
  },
  validationLevel: "strict",
  validationAction: "error"
});

db.runCommand({
  collMod: "files",
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: ["_id", "folderId", "userId", "name", "metadata", "gridFsFileId"],
      properties: {
        _id: {
          bsonType: "binData",
          pattern: "^[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$"
        },
        folderId: {
          bsonType: "binData",
          pattern: "^[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$"
        },
        userId: {
          bsonType: "binData",
          pattern: "^[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$"
        },
        name: {
          bsonType: "string",
          minLength: 1,
          maxLength: 255
        },
        metadata: {
          bsonType: "object",
          required: ["mimeType", "size", "checksum", "created"],
          properties: {
            mimeType: { bsonType: "string" },
            size: { bsonType: "long", minimum: 0 },
            checksum: {
              bsonType: "string",
              pattern: "^sha256:[a-fA-F0-9]{64}$"
            },
            created: { bsonType: "date" },
            updated: { bsonType: "date" }
          }
        },
        gridFsFileId: { bsonType: "objectId" }
      }
    }
  },
  validationLevel: "strict",
  validationAction: "error"
});

// Создание тестовых данных - из data.js
function generateUUID() {
    return UUID();
}

function generateISODate(daysAgo = 0) {
    const date = new Date();
    date.setDate(date.getDate() - daysAgo);
    return date;
}

function generateChecksum() {
    const hexChars = "0123456789abcdef";
    let hash = "";
    for (let i = 0; i < 64; i++) {
        hash += hexChars[Math.floor(Math.random() * 16)];
    }
    return `sha256:${hash}`;
}

const userIds = [
    generateUUID(),
    generateUUID(),
    generateUUID(),
    generateUUID(),
    generateUUID()
];

const folders = [
    {
        _id: generateUUID(),
        userId: userIds[0],
        name: "Рабочие документы",
        metadata: {
            created: generateISODate(30),
            updated: generateISODate(5)
        }
    },
    {
        _id: generateUUID(),
        userId: userIds[0],
        name: "Очень личные файлы",
        metadata: {
            created: generateISODate(28),
            updated: generateISODate(10)
        }
    },
    {
        _id: generateUUID(),
        userId: userIds[0],
        name: "ВУЗовские проекты 2026",
        metadata: {
            created: generateISODate(25),
            updated: generateISODate(2)
        }
    },
    {
        _id: generateUUID(),
        userId: userIds[1],
        name: "Отчеты по ЛР ВУЗа",
        metadata: {
            created: generateISODate(20),
            updated: generateISODate(1)
        }
    },
    {
        _id: generateUUID(),
        userId: userIds[1],
        name: "Презентации",
        metadata: {
            created: generateISODate(18),
            updated: generateISODate(3)
        }
    },
    {
        _id: generateUUID(),
        userId: userIds[2],
        name: "Фотографии",
        metadata: {
            created: generateISODate(15),
            updated: generateISODate(7)
        }
    },
    {
        _id: generateUUID(),
        userId: userIds[2],
        name: "Скачанные видео из Youtube",
        metadata: {
            created: generateISODate(12),
            updated: generateISODate(4)
        }
    },
    {
        _id: generateUUID(),
        userId: userIds[3],
        name: "Инструкции по различным КВНам",
        metadata: {
            created: generateISODate(10),
            updated: generateISODate(1)
        }
    },
    {
        _id: generateUUID(),
        userId: userIds[3],
        name: "ИГОРЫ",
        metadata: {
            created: generateISODate(8),
            updated: generateISODate(2)
        }
    },
    {
        _id: generateUUID(),
        userId: userIds[4],
        name: "Софт для старых компов",
        metadata: {
            created: generateISODate(5),
            updated: generateISODate(1)
        }
    },
    {
        _id: generateUUID(),
        userId: userIds[4],
        name: "Сертификаты",
        metadata: {
            created: generateISODate(3),
            updated: generateISODate(1)
        }
    },
    {
        _id: generateUUID(),
        userId: userIds[0],
        name: "Драйвера",
        metadata: {
            created: generateISODate(60),
            updated: generateISODate(30)
        }
    }
];

db.folders.insertMany(folders);

const fileDefinitions = [
    { folderIndex: 0, userIndex: 0, name: "Техническое_задание.docx", mimeType: "application/vnd.openxmlformats-officedocument.wordprocessingml.document", size: 1548672 },
    { folderIndex: 0, userIndex: 0, name: "Смета_проекта.xlsx", mimeType: "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", size: 892416 },
    { folderIndex: 1, userIndex: 0, name: "Паспорт.pdf", mimeType: "application/pdf", size: 2458624 },
    { folderIndex: 2, userIndex: 0, name: "План_развития.pptx", mimeType: "application/vnd.openxmlformats-officedocument.presentationml.presentation", size: 5242880 },
    { folderIndex: 3, userIndex: 1, name: "Отчет_Q1_2026.pdf", mimeType: "application/pdf", size: 3145728 },
    { folderIndex: 4, userIndex: 1, name: "Презентация_для_клиента.pptx", mimeType: "application/vnd.openxmlformats-officedocument.presentationml.presentation", size: 8388608 },
    { folderIndex: 5, userIndex: 2, name: "IMG_20260315_143022.jpg", mimeType: "image/jpeg", size: 4194304 },
    { folderIndex: 6, userIndex: 2, name: "vacation_2025.mp4", mimeType: "video/mp4", size: 157286400 },
    { folderIndex: 7, userIndex: 3, name: "contract_alpha.pdf", mimeType: "application/pdf", size: 1048576 },
    { folderIndex: 8, userIndex: 3, name: "invoice_2026_03.xlsx", mimeType: "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", size: 524288 },
    { folderIndex: 9, userIndex: 4, name: "course_certificate.pdf", mimeType: "application/pdf", size: 786432 },
    { folderIndex: 10, userIndex: 4, name: "python_advanced_cert.pdf", mimeType: "application/pdf", size: 655360 },
    { folderIndex: 11, userIndex: 0, name: "archive_2025.zip", mimeType: "application/zip", size: 52428800 },
    { folderIndex: 0, userIndex: 0, name: "meeting_notes.txt", mimeType: "text/plain", size: 8192 },
    { folderIndex: 5, userIndex: 2, name: "profile_photo.png", mimeType: "image/png", size: 2097152 }
];

const files = [];

for (const def of fileDefinitions) {
    const folder = folders[def.folderIndex];
    const userId = userIds[def.userIndex];
    const fileId = generateUUID();

    files.push({
        _id: fileId,
        folderId: folder._id,
        userId: userId,
        name: def.name,
        metadata: {
            mimeType: def.mimeType,
            size: NumberLong(def.size.toString()),
            checksum: generateChecksum(),
            created: generateISODate(Math.floor(Math.random() * 30) + 1),
            updated: generateISODate(Math.floor(Math.random() * 5))
        },
        gridFsFileId: ObjectId()
    });
}

db.files.insertMany(files);