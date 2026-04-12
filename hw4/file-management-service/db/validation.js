use("file-management");

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