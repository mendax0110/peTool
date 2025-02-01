#include "../include/UTILS/PrintUtils.h"
#include "../include/FILEIO/FileEditor.h"
#include <iostream>
#include <sstream>
#include <cstring>

using namespace FileEditorInternals;

/**
 * @brief Construct a new File Editor:: File Editor object
 * @details Initializes the cursor position to the start of the file
 */

FileEditor::FileEditor() : inputFileOpen(false), outputFileOpen(false), cursorPosition(0, 0)
{
    lines.emplace_back();
}

/**
 * @brief Destroy the File Editor:: File Editor object
 */
FileEditor::~FileEditor()
{
    closeFile();
}

/**
 * @brief Open a file for reading
 * @param fileName The name of the file to open
 * @return true if the file was opened successfully
 * @return false if the file could not be opened
 */
bool FileEditor::openFileForRead(const std::string& fileName)
{
    if (inputFileOpen)
        closeFile();

    inputFileStream.open(fileName, std::ios::in);
    inputFileOpen = inputFileStream.is_open();
    return inputFileOpen;
}

/**
 * @brief Open a file for writing
 * @param fileName The name of the file to open
 * @return true if the file was opened successfully
 * @return false if the file could not be opened
 */
bool FileEditor::openFileForWrite(const std::string& fileName)
{
    if (outputFileOpen)
        closeFile();

    outputFileStream.open(fileName, std::ios::out);
    outputFileOpen = outputFileStream.is_open();
    return outputFileOpen;
}

/**
 * @brief Open a file for appending
 * @param fileName The name of the file to open
 * @return true if the file was opened successfully
 * @return false if the file could not be opened
 */
bool FileEditor::openFileForAppend(const std::string& fileName)
{
    if (outputFileOpen)
        closeFile();

    outputFileStream.open(fileName, std::ios::app);
    outputFileOpen = outputFileStream.is_open();
    return outputFileOpen;
}

/**
 * @brief Close the file
 */
void FileEditor::closeFile()
{
    if (inputFileStream.is_open())
        inputFileStream.close();
    if (outputFileStream.is_open())
        outputFileStream.close();

    inputFileOpen = false;
    outputFileOpen = false;
}

/**
 * @brief Read the contents of the file
 * @return std::string The contents of the file
 */
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
            std::cerr << "Error reading file: " << getErrorMessage() << std::endl;
        }
    }
    return content;
}

/**
 * @brief Write to the file
 * @param content The content to write to the file
 * @return true if the content was written successfully
 * @return false if the content could not be written
 */
bool FileEditor::writeFile(const std::string& content)
{
    if (outputFileOpen)
    {
        outputFileStream << content;
        if (outputFileStream.fail())
        {
            std::cerr << "Error writing file: " << getErrorMessage() << std::endl;
            return false;
        }
        return true;
    }
    return false;
}

/**
 * @brief Append to the file
 * @param content The content to append to the file
 * @return true if the content was appended successfully
 * @return false if the content could not be appended
 */
bool FileEditor::appendToFile(const std::string& content)
{
    if (outputFileOpen)
    {
        outputFileStream << content;
        if (outputFileStream.fail())
        {
            std::cerr << "Error appending to file: " << getErrorMessage() << std::endl;
            return false;
        }
        return true;
    }
    return false;
}

/**
 * @brief Get the size of the file
 * @return size_t The size of the file
 */
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

/**
 * @brief Check if the file is open
 * @return true if the file is open
 * @return false if the file is not open
 */
bool FileEditor::isOpen() const
{
    return inputFileOpen || outputFileOpen;
}

/**
 * @brief Set the text of the file editor
 * @param text The text to set
 */
void FileEditor::SetText(const std::string& text)
{
    lines.clear();
    size_t start = 0;
    size_t end = text.find('\n');
    while (end != std::string::npos)
    {
        lines.push_back(text.substr(start, end - start));
        start = end + 1;
        end = text.find('\n', start);
    }
    lines.push_back(text.substr(start));
    cursorPosition = Position(0, 0);
}

/**
 * @brief Get the text of the file editor
 * @return std::string The text of the file editor
 */
std::string FileEditor::GetText() const
{
    std::string result;
    for (const auto& line : lines)
    {
        result += line + '\n';
    }
    return result;
}

/**
 * @brief Render the file editor
 * @param title The title of the file editor window
 * @param size The size of the file editor window
 * @param border Whether to show the border of the window
 */
void FileEditor::Render(const char* title, const ImVec2& size, bool border)
{
    static std::vector<char> buffer;
    std::string text = GetText();

    if (buffer.size() < text.size() + 1)
    {
        buffer.resize(text.size() + 1);
    }

    if (strncmp(buffer.data(), text.c_str(), text.size()) != 0)
    {
        std::copy(text.begin(), text.end(), buffer.begin());
        buffer[text.size()] = '\0'; // null terminator for end of string
    }

    ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_HorizontalScrollbar | (border ? 0 : ImGuiWindowFlags_NoTitleBar)))
    {
        HandleKeyboardInputs();
        HandleMouseInputs();

        ImVec2 availSize = ImGui::GetContentRegionAvail();
        ImGui::InputTextMultiline("##editor", buffer.data(), buffer.size(), availSize, ImGuiInputTextFlags_AllowTabInput);

        std::string newText(buffer.data());
        if (newText != text)
        {
            SetText(newText);
        }

        if (ImGui::IsWindowFocused())
        {
            ImGui::SetKeyboardFocusHere();
        }

        ImGui::End();
    }
}

/**
 * @brief Get the cursor position
 * @return Position The cursor position
 */
FileEditor::Position FileEditor::GetCursorPosition() const
{
    return cursorPosition;
}

/**
 * @brief Set the cursor position
 * @param pos The position to set the cursor to
 */
void FileEditor::SetCursorPosition(const Position& pos)
{
    if (pos.line >= 0 && pos.line < lines.size()
        && pos.column >= 0 && pos.column <= lines[pos.line].size()) {
        cursorPosition = pos;
    }
}

/**
 * @brief Move the cursor up
 */
void FileEditor::MoveUp()
{
    if (cursorPosition.line > 0)
    {
        cursorPosition.line--;
        cursorPosition.column = std::min<int>(cursorPosition.column, static_cast<int>(lines[cursorPosition.line].size()));
        EnsureCursorVisible();
    }
}

/**
 * @brief Move the cursor down
 */
void FileEditor::MoveDown()
{
    if (cursorPosition.line < lines.size() - 1)
    {
        cursorPosition.line++;
        cursorPosition.column = std::min<int>(cursorPosition.column, static_cast<int>(lines[cursorPosition.line].size()));
        EnsureCursorVisible();
    }
}

/**
 * @brief Move the cursor left
 */
void FileEditor::MoveLeft()
{
    if (cursorPosition.column > 0)
    {
        cursorPosition.column--;
    }
    else if (cursorPosition.line > 0)
    {
        cursorPosition.line--;
        cursorPosition.column = static_cast<int>(lines[cursorPosition.line].size());
    }
    EnsureCursorVisible();
}

/**
 * @brief Move the cursor right
 */
void FileEditor::MoveRight()
{
    if (cursorPosition.column < lines[cursorPosition.line].size())
    {
        cursorPosition.column++;
    }
    else if (cursorPosition.line < lines.size() - 1)
    {
        cursorPosition.line++;
        cursorPosition.column = 0;
    }
    EnsureCursorVisible();
}

/**
 * @brief Insert text at the cursor position
 * @param text The text to insert
 */
void FileEditor::InsertText(const std::string &text)
{
    auto& line = lines[cursorPosition.line];
    line.insert(cursorPosition.column, text);
    cursorPosition.column += static_cast<int>(text.size());
}

/**
 * @brief Delete text at the cursor position
 */
void FileEditor::DeleteText()
{
    auto& line = lines[cursorPosition.line];
    if (cursorPosition.column < line.size())
    {
        line.erase(cursorPosition.column, 1);
    }
    else if (cursorPosition.line < lines.size() - 1)
    {
        line += lines[cursorPosition.line + 1];
        lines.erase(lines.begin() + cursorPosition.line + 1);
    }
}

/**
 * @brief Copy the text at the cursor position
 */
void FileEditor::Copy()
{
    ImGui::SetClipboardText(lines[cursorPosition.line].c_str());
}

/**
 * @brief Cut the text at the cursor position
 */
void FileEditor::Cut()
{
    ImGui::SetClipboardText(lines[cursorPosition.line].c_str());
    lines[cursorPosition.line].clear();
}

/**
 * @brief Paste text at the cursor position
 */
void FileEditor::Paste()
{
    auto clipboardText = ImGui::GetClipboardText();
    if (clipboardText)
    {
        InsertText(clipboardText);
    }
}

/**
 * @brief Push the current state of the file editor to the undo stack
 */
void FileEditor::PushUndo()
{
    undoStack.push_back({GetText(), cursorPosition});
    redoStack.clear();
}

/**
 * @brief Undo the last edit action
 */
void FileEditor::Undo()
{
    if (!undoStack.empty())
    {
        redoStack.push_back({GetText(), cursorPosition});
        ApplyEditAction(undoStack.back());
        undoStack.pop_back();
    }
}

/**
 * @brief Redo the last edit action
 */
void FileEditor::Redo()
{
    if (!redoStack.empty())
    {
        undoStack.push_back({GetText(), cursorPosition});
        ApplyEditAction(redoStack.back());
        redoStack.pop_back();
    }
}

/**
 * @brief Apply an edit action
 * @param action The edit action to apply
 */
void FileEditor::ApplyEditAction(const EditAction& action)
{
    SetText(action.content);
    SetCursorPosition(action.cursorPos);
}

/**
 * @brief Ensure the cursor is visible
 */
void FileEditor::EnsureCursorVisible()
{
    if (cursorPosition.line < 0)
    {
        cursorPosition.line = 0;
    }
    else if (cursorPosition.line >= lines.size())
    {
        cursorPosition.line = static_cast<int>(lines.size() - 1);
    }

    if (cursorPosition.column < 0)
    {
        cursorPosition.column = 0;
    }
    else if (cursorPosition.column > lines[cursorPosition.line].size())
    {
        cursorPosition.column = static_cast<int>(lines[cursorPosition.line].size());
    }
}

/**
 * @brief Handle keyboard inputs
 */
void FileEditor::HandleKeyboardInputs()
{
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
    {
        MoveUp();
    }
    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
    {
        MoveDown();
    }
    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
    {
        MoveLeft();
    }
    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
    {
        MoveRight();
    }
    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)) && ImGui::GetIO().KeyCtrl)
    {
        Copy();
    }
    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_X)) && ImGui::GetIO().KeyCtrl)
    {
        Cut();
    }
    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V)) && ImGui::GetIO().KeyCtrl)
    {
        Paste();
    }
    else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Backspace)))
    {
        DeleteText();
    }
}

/**
 * @brief Handle mouse inputs
 */
void FileEditor::HandleMouseInputs()
{
    if (ImGui::IsItemHovered())
    {
        if (ImGui::IsMouseClicked(0))
        {
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 textPos = ImGui::GetCursorScreenPos();
            ImVec2 relativePos = ImVec2(mousePos.x - textPos.x, mousePos.y - textPos.y);
            int line = static_cast<int>(relativePos.y / ImGui::GetTextLineHeight());
            int column = 0;
            int currentLine = 0;
            for (const auto& currentLineText : lines)
            {
                if (currentLine == line)
                {
                    break;
                }
                column += static_cast<int>(currentLineText.size());
                ++currentLine;
            }
            SetCursorPosition(Position(line, column));
        }
    }
}