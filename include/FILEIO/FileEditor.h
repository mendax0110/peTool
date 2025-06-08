#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "imgui.h"

/// @brief FileEditorInternals namespace, which contains the FileEditor class for editing files with a GUI \namespace FileEditorInternals
namespace FileEditorInternals
{
    /// @brief The FileEditor class, a class for editing files with a GUI using ImGui \class FileEditor
    class FileEditor
    {
    public:

        /// @brief Position struct to represent the cursor position in the file editor \struct Position
        struct Position
        {
            int line;
            int column;
            Position(int l = 0, int c = 0) : line(l), column(c) {}
            bool operator==(const Position& other) const { return line == other.line && column == other.column; }
            bool operator!=(const Position& other) const { return !(*this == other); }
        };

        /**
         * @brief Construct a new FileEditor object
         */
        FileEditor();

        /**
         * @brief Destroy the FileEditor object
         */
        ~FileEditor();

        /**
         * @brief Set the text content of the file editor
         * @param text The text to set
         */
        void SetText(const std::string& text);

        /**
         * @brief Get the text content of the file editor
         * @return The text content
         */
        std::string GetText() const;

        /**
         * @brief Render the file editor with a title and size
         * @param title The title of the editor
         * @param size The size of the editor window
         * @param border Whether to draw a border around the editor
         */
        void Render(const char* title, const ImVec2& size, bool border = true);

        /**
         * @brief Get the current cursor position
         * @return The current cursor position
         */
        Position GetCursorPosition() const;

        /**
         * @brief Set the cursor position
         * @param pos The position to set the cursor to
         */
        void SetCursorPosition(const Position& pos);

        /**
         * @brief Move the cursor to the beginning of the file
         */
        void MoveUp();

        /**
         * @brief Move the cursor to the end of the file
         */
        void MoveDown();

        /**
         * @brief Move the cursor to the left
         */
        void MoveLeft();

        /**
         * @brief Move the cursor to the right
         */
        void MoveRight();

        /**
         * @brief Insert text at the current cursor position
         * @param text The text to insert
         */
        void InsertText(const std::string& text);

        /**
         * @brief Delete text at the current cursor position
         */
        void DeleteText();

        /**
         * @brief Copies the selected text to the clipboard
         */
        void Copy();

        /**
         * @brief Cuts the selected text and copies it to the clipboard
         */
        void Cut();

        /**
         * @brief Pastes the text from the clipboard at the current cursor position
         */
        void Paste();

        /**
         * @brief Opens a file for reading.
         * @param fileName The name of the file to open.
         * @return A boolean indicating whether the file was successfully opened.
         */
        bool openFileForRead(const std::string& fileName);

        /**
         * @brief Opens a file for writing.
         * @param fileName The name of the file to open.
         * @return A boolean indicating whether the file was successfully opened.
         */
        bool openFileForWrite(const std::string& fileName);

        /**
         * @brief Opens a file for appending.
         * @param fileName The name of the file to open.
         * @return A boolean indicating whether the file was successfully opened.
         */
        bool openFileForAppend(const std::string& fileName);

        /**
         * @brief Closes the currently opened file.
         */
        void closeFile();

        /**
         * @brief Reads the content of the currently opened file.
         * @return The content of the file as a string.
         */
        std::string readFile();

        /**
         * @brief Undo the last edit action.
         */
        void Undo();

        /**
         * @brief Redo the last undone edit action.
         */
        void Redo();

        /**
         * @brief Writes the current content to the file.
         * @param content The content to write to the file.
         * @return A boolean indicating whether the write operation was successful.
         */
        bool writeFile(const std::string& content);

        /**
         * @brief Appends content to the currently opened file.
         * @param content The content to append to the file.
         * @return A boolean indicating whether the append operation was successful.
         */
        bool appendToFile(const std::string& content);

        /**
         * @brief Gets the size of the currently opened file.
         * @return The size of the file in bytes.
         */
        size_t getFileSize();

        /**
         * @brief Checks if the file editor is currently open.
         * @return A boolean indicating whether the file editor is open.
         */
        bool isOpen() const;

    private:
        std::fstream inputFileStream;
        std::fstream outputFileStream;
        std::vector<std::string> lines;
        Position cursorPosition;

        /// @brief Struct to hold the content and cursor position for undo/redo actions \struct EditActionb
        struct EditAction
        {
            std::string content;
            Position cursorPos;
        };

        std::vector<EditAction> undoStack;
        std::vector<EditAction> redoStack;

        bool inputFileOpen;
        bool outputFileOpen;

        /**
         * @brief Ensure the cursor is visible in the editor
         */
        void EnsureCursorVisible();

        /**
         * @brief Handle keyboard inputs for the file editor
         */
        void HandleKeyboardInputs();

        /**
         * @brief Handle mouse inputs for the file editor
         */
        void HandleMouseInputs();

        /**
         * @brief Push the current state onto the undo stack
         */
        void PushUndo();

        /**
         * @brief Push the current state onto the redo stack
         */
        void ApplyEditAction(const EditAction& action);
    };
}