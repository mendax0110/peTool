#include "../include/VIEW/GraphView.h"
#include <iostream>

GraphView::GraphView() : nodes(), connections()
{
}


void GraphView::AddNode(int id, const ImVec2& position, const ImVec2& size, const std::string& name, const ImVec4& color)
{
    Node newNode{id, position, size, color, name};
    nodes.push_back(newNode);
}

void GraphView::AddConnection(int sourceNodeId, int targetNodeId)
{
    Connection newConnection{sourceNodeId, targetNodeId};
    connections.push_back(newConnection);
}

const Node* GraphView::FindNodeById(int id) const
{
    for (const auto& node : nodes)
    {
        if (node.id == id)
        {
            return &node;
        }
    }
    return nullptr;
}

void GraphView::Render()
{
    ImGui::Begin("Graph View", nullptr, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& node : nodes)
    {
        ImGui::SetCursorPos(node.position);
        ImGui::PushID(node.id);
        ImGui::PushStyleColor(ImGuiCol_Button, node.color);
        ImGui::Button(node.name.c_str(), node.size);
        ImGui::PopStyleColor();
        ImGui::PopID();
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    for (const auto& connection : connections)
    {
        const Node* sourceNode = FindNodeById(connection.sourceNodeId);
        const Node* targetNode = FindNodeById(connection.targetNodeId);

        if (sourceNode && targetNode)
        {
            ImVec2 sourcePos = ImVec2(sourceNode->position.x + sourceNode->size.x / 2, sourceNode->position.y + sourceNode->size.y / 2);
            ImVec2 targetPos = ImVec2(targetNode->position.x + targetNode->size.x / 2, targetNode->position.y + targetNode->size.y / 2);
            drawList->AddLine(sourcePos, targetPos, IM_COL32(255, 255, 255, 255), 2.0f);
        }
    }

    ImGui::End();
}