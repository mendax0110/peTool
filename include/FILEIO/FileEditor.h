#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "imgui.h"

namespace FileEditorInternals
{
    class FileEditor
    {
    public:
        struct Position
        {
            int line;
            int column;
            Position(int l = 0, int c = 0) : line(l), column(c) {}
            bool operator==(const Position& other) const { return line == other.line && column == other.column; }
            bool operator!=(const Position& other) const { return !(*this == other); }
        };

        FileEditor();
        ~FileEditor();

        void SetText(const std::string& text);
        std::string GetText() const;
        void Render(const char* title, const ImVec2& size, bool border = true);
        Position GetCursorPosition() const;
        void SetCursorPosition(const Position& pos);

        void MoveUp();
        void MoveDown();
        void MoveLeft();
        void MoveRight();

        void InsertText(const std::string& text);
        void DeleteText();

        void Copy();
        void Cut();
        void Paste();

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
        std::vector<std::string> lines;
        Position cursorPosition;

        bool inputFileOpen;
        bool outputFileOpen;

        void EnsureCursorVisible();
        void HandleKeyboardInputs();
        void HandleMouseInputs();
    };
}