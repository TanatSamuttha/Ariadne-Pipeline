#pragma once

#include <functional>
#include <string>
#include <vector>
#include <queue>
#include <map>

template<typename T>
class Pipeline
{
private:
    std::vector<std::function<std::string(T*)>> nodes;
    std::map<std::string, size_t> nodeIds;
    std::vector<size_t> inDegrees;
    std::vector<stze_t>outDegrees;
    std::vector<size_t> remainInDegrees;
    std::vector<std::vector<size_t>> adj;
    std::queue<size_t> tasks;

    size_t concurrenting;
    bool initedAdj;
    
    std::string dummy (T* state)
    {
        return "";
    }

    void init ()
    {
        nodeIds["End"] = 0;
        nodes.push_back(dummy);
        nodeIds["Start"] = 1;
        nodes.push_back(dummy);
        inDegrees.resize(2);
        outDegrees.resize(2);
        initedAdj = false;
    }

public:
    T* state;

    const std::string START = "Start";
    const std::string END = "End";

    Pipeline ()
    {
        init();
    }

    Pipeline (T* state) : state(state)
    {
        init();
    }

    void addNode (std::string name, std::function<std::string(T*)> callable)
    {
        nodeIds[name] = nodes.size();
        nodes.push_back(callable);
        inDegrees.push_back(0);
        outDegrees.push_back(0);
        remainInDegrees.push_back(0);
        initedAdj = false;
    }

    void addEdge (std::string origin, std::string destination)
    {
        if (!initedAdj)
            adj = std::vector<std::vector<size_t>> (nodes.size());

        size_t originId = nodeIds[origin], destinationId = nodeIds[destination]
        adj[originId].push_back(destinationId);
        ++inDegrees[destinationId];
        ++remainInDegrees[destinationId];
    }

    void exec ()
    {
        tasks.push(nodeIds[START]);
    }
};