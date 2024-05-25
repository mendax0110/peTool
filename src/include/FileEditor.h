#include <iostream>
#include <fstream>
#include <string>

namespace FileEditorInternals
{
    class FileEditor
    {
    public:
        FileEditor();
        ~FileEditor();

        bool openFileForRead(const std::string& fileName);
        bool openFileForWrite(const std::string& fileName);
        bool openFileForAppend(const std::string& fileName);
        void closeFile();
        std::string readFile();

        bool writeFile(const std::string& content);
        bool appendToFile(const std::string& content);
        size_t getFileSize();
        bool isOpen() const;
    private:
        std::fstream inputFileStream;
        std::fstream outputFileStream;
        bool inputFileOpen;
        bool outputFileOpen;
    };
}