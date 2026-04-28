use("file-management");

const userId = UUID("550e8400-e29b-41d4-a716-446655440000");

const folderId = UUID();
// Создание новой папки
db.folders.insertOne({
  _id: folderId,
  userId: userId,
  name: "Новая папка",
  metadata: {
    created: new Date(),
    updated: new Date()
  }
});

const fileId = UUID();
// Создание нового файла
db.files.insertOne({
  _id: fileId,
  folderId: folderId,
  userId: userId,
  name: "document.pdf",
  metadata: {
    mimeType: "application/pdf",
    size: NumberLong("2458624"),
    checksum: "sha256:a1b2c3d4e5f6789012345678901234567890123456789012345678901234abcd",
    created: new Date(),
    updated: new Date()
  },
  gridFsFileId: ObjectId()
});

// Создание еще одного нового файла
db.files.insertOne({
  _id: UUID(),
  folderId: folderId,
  userId: userId,
  name: "document_another.pdf",
  metadata: {
    mimeType: "application/pdf",
    size: NumberLong("3450457"),
    checksum: "sha256:0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
    created: new Date(),
    updated: new Date()
  },
  gridFsFileId: ObjectId()
});

// Получение всех папок пользователя
db.folders.find({
  userId: userId
}).toArray();

// Получение одной папки по ID
db.folders.findOne({
  _id: folderId
});

// Получение всех файлов в папке по ID папки
db.files.find({
  folderId: folderId
}).toArray();

// Поиск файла по имени в папке
db.files.findOne({
  folderId: folderId,
  name: "document.pdf"
});

// Поиск среди всех папок с именем (равно)
db.folders.find({
  name: { $eq: "Новая папка" }
}).toArray();

// Поиск среди всех папок с именем (не равно)
db.folders.find({
  name: { $ne: "Новая папка" }
}).toArray();

// Поиск среди всех файлов с размером больше 1MB
db.files.find({
  "metadata.size": { $gt: NumberLong("1048576") }
}).toArray();

// Поиск среди всех файлов с размером меньше 100KB
db.files.find({
  "metadata.size": { $lt: NumberLong("102400") }
}).toArray();

// Поиск среди всех файлов с размером в диапазоне от 1MB до 10MB
db.files.find({
  "metadata.size": {
    $gte: NumberLong("1048576"),
    $lte: NumberLong("10485760")
  }
}).toArray();

// Поиск среди всех файлов по списку MIME-типов
db.files.find({
  "metadata.mimeType": {
    $in: ["application/pdf", "image/jpeg", "image/png"]
  }
}).toArray();

// Поиск среди всех файлов вне списка MIME-типов
db.files.find({
  "metadata.mimeType": {
    $nin: ["application/zip", "application/x-rar"]
  }
}).toArray();

// Поиск файлов с оператором AND - пользователь и файл больше 1 МБ
db.files.find({
  $and: [
    { userId: userId },
    { "metadata.size": { $gt: NumberLong("1048576") } }
  ]
}).toArray();

// Поиск среди всех папок с оператором OR
db.folders.find({
  $or: [
    { name: "Рабочие документы" },
    { name: "Очень личные файлы" }
  ]
}).toArray();

// Поиск среди всех папок по шаблону имени REGEX
db.folders.find({
  name: { $regex: /^Проект.*/i }
}).toArray();

// Поиск среди всех папок где поле updated существует
db.folders.find({
  "metadata.updated": { $exists: true }
}).toArray();

// Получение всех папок с сортировкой по дате создания
db.folders.find().sort({ "metadata.created": -1 }).toArray();

// Получение всех файлов с пагинацией
db.files.find().skip(10).limit(5).toArray();

// Агрегация: статистика по пользователю
db.files.aggregate([
  { $match: { userId: userId } },
  {
    $group: {
      _id: "$userId",
      totalFiles: { $sum: 1 },
      totalSize: { $sum: "$metadata.size" },
      avgFileSize: { $avg: "$metadata.size" }
    }
  }
]).toArray();

// Агрегация: топ MIME-типов среди всех файлов всех пользователей
db.files.aggregate([
  {
    $group: {
      _id: "$metadata.mimeType",
      count: { $sum: 1 },
      totalSize: { $sum: "$metadata.size" }
    }
  },
  { $sort: { count: -1 } },
  { $limit: 5 }
]).toArray();

const newFolderName = "Новое имя папки";
// Обновление имени папки
db.folders.updateOne(
  { _id: folderId },
  {
    $set: {
      name: newFolderName,
      "metadata.updated": new Date()
    }
  }
);

const newFileName = "Новое имя файла";
// Обновление имени файла
db.files.updateOne(
  { _id: fileId },
  {
    $set: {
      name: newFileName,
      "metadata.updated": new Date()
    }
  }
);

// Обновление метаданных файла
db.files.updateOne(
  { _id: fileId },
  {
    $set: {
      "metadata.mimeType": "application/pdf",
      "metadata.checksum": "sha256:8f434346648f6b96df89dda901c5176b10a6d83961dd3c1ac88b59b2dc327aa4",
      "metadata.size": NumberLong("4839274249"),
      "metadata.updated": new Date()
    }
  }
);

// Обновление с удалением поля updated у папки
db.folders.updateOne(
  { _id: folderId },
  {
    $unset: {
      "metadata.updated": ""
    }
  }
);

// Удаление файла по ID
db.files.deleteOne({
  _id: fileId
});

// Удаление всех файлов в папке с ID
db.files.deleteMany({
  folderId: folderId
});

// Удаление папки по ID
db.folders.deleteOne({
  _id: folderId
});

// Удаление всех папок пользователя
db.folders.deleteMany({
  userId: userId
});

// Удаление всех файлов всех пользователей размером больше 100MB
db.files.deleteMany({
  "metadata.size": { $gt: NumberLong("104857600") }
});

// Удаление всех файлов всех пользователей, если их MIME-тип один из [application/zip, application/x-rar]
db.files.deleteMany({
  "metadata.mimeType": { $in: ["application/zip", "application/x-rar"] }
});