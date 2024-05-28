#pragma once
#include "imgui.h"
#include <vector>
#include <string>

struct Connection
{
    int sourceNodeId;
    int targetNodeId;
};

struct Node
{
    int id;
    ImVec2 position;
    ImVec2 size;
    ImVec4 color;
    std::string name;
};

class GraphView
{
public:
    GraphView();

    void AddNode(int id, const ImVec2& position, const ImVec2& size, const std::string& name, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    void AddConnection(int sourceNodeId, int targetNodeId);
    void Render();
    const Node* FindNodeById(int id) const;

private:
    std::vector<Node> nodes;
    std::vector<Connection> connections;
};