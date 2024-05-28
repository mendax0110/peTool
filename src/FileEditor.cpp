#include "../include/FILEIO/FileEditor.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

using namespace FileEditorInternals;

FileEditor::FileEditor() : inputFileOpen(false), outputFileOpen(false)
{
}

FileEditor::~FileEditor()
{
    closeFile();
}

bool FileEditor::openFileForRead(const std::string& fileName)
{
    if (inputFileOpen)
        closeFile();

    inputFileStream.open(fileName, std::ios::in);
    inputFileOpen = inputFileStream.is_open();
    return inputFileOpen;
}

bool FileEditor::openFileForWrite(const std::string& fileName)
{
    if (outputFileOpen)
        closeFile();

    outputFileStream.open(fileName, std::ios::out);
    outputFileOpen = outputFileStream.is_open();
    return outputFileOpen;
}

bool FileEditor::openFileForAppend(const std::string& fileName)
{
    if (outputFileOpen)
        closeFile();

    outputFileStream.open(fileName, std::ios::app);
    outputFileOpen = outputFileStream.is_open();
    return outputFileOpen;
}

void FileEditor::closeFile()
{
    if (inputFileStream.is_open())
        inputFileStream.close();
    if (outputFileStream.is_open())
        outputFileStream.close();

    inputFileOpen = false;
    outputFileOpen = false;
}

std::string FileEditor::readFile()
{
    std::string content;
    if (inputFileOpen)
    {
        std::ostringstream ss;
        ss << inputFileStream.rdbuf();
        content = ss.str();

        if (inputFileStream.fail() && !inputFileStream.eof())
        {
            std::cerr << "Error reading file: " << strerror(errno) << std::endl;
        }
    }
    return content;
}

bool FileEditor::writeFile(const std::string& content)
{
    if (outputFileOpen)
    {
        outputFileStream << content;
        if (outputFileStream.fail())
        {
            std::cerr << "Error writing file: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }
    return false;
}

bool FileEditor::appendToFile(const std::string& content)
{
    if (outputFileOpen)
    {
        outputFileStream << content;
        if (outputFileStream.fail())
        {
            std::cerr << "Error appending to file: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }
    return false;
}

size_t FileEditor::getFileSize()
{
    if (outputFileOpen)
    {
        std::streampos begin, end;
        begin = outputFileStream.tellp();
        outputFileStream.seekp(0, std::ios::end);
        end = outputFileStream.tellp();
        outputFileStream.seekp(0, std::ios::beg);
        return static_cast<size_t>(end - begin);
    }
    return 0;
}

bool FileEditor::isOpen() const
{
    return inputFileOpen || outputFileOpen;
}