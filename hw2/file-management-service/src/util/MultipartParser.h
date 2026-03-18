#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

#include <Poco/Net/MultipartReader.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/StreamCopier.h>

using namespace std;

namespace maxdisk::filemanagement::util
{
    class MultipartParser
    {
    public:
        struct FilePart
        {
            string fieldName;
            string fileName;
            string contentType;
            vector<uint8_t> content;
        };

        static unordered_map<string, FilePart> parse(istream &inputStream, const string &contentType)
        {
            unordered_map<string, FilePart> result;
            try
            {
                Poco::Net::MultipartReader reader(inputStream);

                while (reader.hasNextPart())
                {
                    Poco::Net::MessageHeader header;
                    reader.nextPart(header);
                    FilePart part;
                    auto disposition = header.get("Content-Disposition", "");

                    auto namePos = disposition.find("name=\"");
                    if (namePos != string::npos)
                    {
                        namePos += 6;
                        auto nameEnd = disposition.find('"', namePos);
                        if (nameEnd != string::npos)
                        {
                            part.fieldName = disposition.substr(namePos, nameEnd - namePos);
                        }
                    }

                    auto filePos = disposition.find("filename=\"");
                    if (filePos != string::npos)
                    {
                        filePos += 10;
                        auto fileEnd = disposition.find('"', filePos);
                        if (fileEnd != string::npos)
                        {
                            part.fileName = disposition.substr(filePos, fileEnd - filePos);
                        }
                    }

                    part.contentType = header.get("Content-Type", "application/octet-stream");

                    ostringstream oss;
                    Poco::StreamCopier::copyStream(reader.stream(), oss);
                    const string &data = oss.str();
                    part.content.assign(data.begin(), data.end());

                    if (!part.fieldName.empty())
                    {
                        result[part.fieldName] = move(part);
                    }
                }
            }
            catch (const Poco::Exception &exc)
            {
                throw runtime_error("Ошибка парсинга Multipart: " + exc.displayText());
            }

            return result;
        }
    };

}