#pragma once
#include "imgui.h"
#include <vector>
#include <string>


/// @brief Represents a connection between two nodes in the graph view \struct Connection
struct Connection
{
    int sourceNodeId;
    int targetNodeId;
};

/// @brief Represents a node in the graph view \struct Node
struct Node
{
    int id;
    ImVec2 position;
    ImVec2 size;
    ImVec4 color;
    std::string name;
};

/// @brief GraphView class, which provides methods for rendering a graph view with nodes and connections \class GraphView
class GraphView
{
public:

    /**
     * @brief Construct a new GraphView object
     */
    GraphView();

    /**
     * @brief Adds a node to the graph view.
     * @param id The unique identifier for the node.
     * @param position The position of the node in the graph view.
     * @param size The size of the node.
     * @param name The name of the node.
     * @param color A color for the node, defaulting to white.
     */
    void AddNode(int id, const ImVec2& position, const ImVec2& size, const std::string& name, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    /**
     * @brief Adds a connection between two nodes in the graph view.
     * @param sourceNodeId The unique identifier of the source node.
     * @param targetNodeId The unique identifier of the target node.
     */
    void AddConnection(int sourceNodeId, int targetNodeId);

    /**
     * @brief Renders the graph view with nodes and connections.
     */
    void Render();

    /**
     * @brief Finds a node by its unique identifier.
     * @param id The unique identifier of the node to find.
     * @return A pointer to the found node, or nullptr if not found.
     */
    const Node* FindNodeById(int id) const;

private:
    std::vector<Node> nodes;
    std::vector<Connection> connections;
};