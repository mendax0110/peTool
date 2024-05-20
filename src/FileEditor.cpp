#include "./include/FileEditor.h"

using namespace FileEditorInternals;

FileEditor::FileEditor() : fileOpen(false)
{
}

FileEditor::~FileEditor()
{
    closeFile();
}

bool FileEditor::openFileForRead(const std::string& fileName)
{
    if (fileOpen)
        closeFile();

    inputFileStream.open(fileName, std::ios::in);
    fileOpen = inputFileStream.is_open();
    return fileOpen;
}

bool FileEditor::openFileForWrite(const std::string& fileName)
{
    if (fileOpen)
        closeFile();

    outputFileStream.open(fileName, std::ios::out);
    fileOpen = outputFileStream.is_open();
    return fileOpen;
}

bool FileEditor::openFileForAppend(const std::string& fileName)
{
    if (fileOpen)
        closeFile();

    outputFileStream.open(fileName, std::ios::app);
    fileOpen = outputFileStream.is_open();
    return fileOpen;
}

void FileEditor::closeFile()
{
    if (inputFileStream.is_open())
        inputFileStream.close();
    if (outputFileStream.is_open())
        outputFileStream.close();
    fileOpen = false;
}

std::string FileEditor::readFile()
{
    std::string content;
    if (fileOpen)
    {
        std::string line;
        while (std::getline(inputFileStream, line))
            content += line + "\n";

        if (!inputFileStream.eof() && inputFileStream.fail())
        {
            std::cerr << "Error reading file: " << strerror(errno) << std::endl;
        }
    }
    return content;
}

bool FileEditor::writeFile(const std::string& content)
{
    if (fileOpen)
    {
        outputFileStream << content;
        return true;
    }
    return false;
}

bool FileEditor::appendToFile(const std::string& content)
{
    if (fileOpen)
    {
        outputFileStream << content;
        return true;
    }
    return false;
}

size_t FileEditor::getFileSize()
{
    if (fileOpen)
    {
        std::streampos begin, end;
        begin = outputFileStream.tellg();
        outputFileStream.seekg(0, std::ios::end);
        end = outputFileStream.tellg();
        outputFileStream.seekg(0, std::ios::beg);
        return static_cast<size_t>(end - begin);
    }
    return 0;
}

bool FileEditor::isOpen() const
{
    return fileOpen;
}