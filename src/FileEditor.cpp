#include "../include/FILEIO/FileEditor.h"
#include <iostream>
#include <sstream>
#include <cstring>

using namespace FileEditorInternals;

FileEditor::FileEditor() : inputFileOpen(false), outputFileOpen(false), cursorPosition(0, 0)
{
    lines.emplace_back();
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

std::string FileEditor::GetText() const
{
    std::string result;
    for (const auto& line : lines)
    {
        result += line + '\n';
    }
    return result;
}

void FileEditor::Render(const char* title, const ImVec2& size, bool border)
{
    if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_HorizontalScrollbar | (border ? 0 : ImGuiWindowFlags_NoTitleBar)))
    {
        HandleKeyboardInputs();
        HandleMouseInputs();

        std::string text = GetText();
        char* buffer = &text[0];
        ImVec2 availSize = ImGui::GetContentRegionAvail();
        ImGui::InputTextMultiline("##editor", buffer, text.size() + 1, availSize, ImGuiInputTextFlags_AllowTabInput);
        SetText(buffer);

        if (ImGui::IsWindowFocused())
        {
            ImGui::SetKeyboardFocusHere();
        }
    }
    ImGui::End();
}

FileEditor::Position FileEditor::GetCursorPosition() const
{
    return cursorPosition;
}

void FileEditor::SetCursorPosition(const Position& pos)
{
    if (pos.line >= 0 && pos.line < lines.size()
        && pos.column >= 0 && pos.column <= lines[pos.line].size()) {
        cursorPosition = pos;
    }
}

void FileEditor::MoveUp()
{
    if (cursorPosition.line > 0)
    {
        cursorPosition.line--;
        cursorPosition.column = std::min(cursorPosition.column, static_cast<int>(lines[cursorPosition.line].size()));
        EnsureCursorVisible();
    }
}

void FileEditor::MoveDown()
{
    if (cursorPosition.line < lines.size() - 1)
    {
        cursorPosition.line++;
        cursorPosition.column = std::min(cursorPosition.column, static_cast<int>(lines[cursorPosition.line].size()));
        EnsureCursorVisible();
    }
}

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

void FileEditor::InsertText(const std::string &text)
{
    auto& line = lines[cursorPosition.line];
    line.insert(cursorPosition.column, text);
    cursorPosition.column += static_cast<int>(text.size());
}

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

void FileEditor::Copy()
{
    ImGui::SetClipboardText(lines[cursorPosition.line].c_str());
}

void FileEditor::Cut()
{
    ImGui::SetClipboardText(lines[cursorPosition.line].c_str());
    lines[cursorPosition.line].clear();
}

void FileEditor::Paste()
{
    auto clipboardText = ImGui::GetClipboardText();
    if (clipboardText)
    {
        InsertText(clipboardText);
    }
}

void FileEditor::PushUndo()
{
    undoStack.push_back({GetText(), cursorPosition});
    redoStack.clear();
}

void FileEditor::Undo()
{
    if (!undoStack.empty())
    {
        redoStack.push_back({GetText(), cursorPosition});
        ApplyEditAction(undoStack.back());
        undoStack.pop_back();
    }
}

void FileEditor::Redo()
{
    if (!redoStack.empty())
    {
        undoStack.push_back({GetText(), cursorPosition});
        ApplyEditAction(redoStack.back());
        redoStack.pop_back();
    }
}

void FileEditor::ApplyEditAction(const EditAction& action)
{
    SetText(action.content);
    SetCursorPosition(action.cursorPos);
}

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