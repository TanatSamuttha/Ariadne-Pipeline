#pragma once

#include <functional>
#include <string>
#include <vector>
#include <map>

template<typename T>
class Pipeline
{
private:
    std::vector<std::function<string(T*)>> node;
    std::map<std::string, size_t> nodeId;
    
    string dummy (T* state)
    {
        return "";
    }

    void init ()
    {
        nodeId["End"] = 0;
        node.push_back(dummy);
        nodeId["Start"] = 1;
        node.push_back(dummy);
    }

public:
    T* state;

    Pipeline ()
    {
        init();
    }

    Pipeline (T* state) : state(state)
    {
        init();
    }

    void addNode (std::string name, std::function<string(T*)> callable)
    {
        nodeId[name] = node.size();
        node.push_back(callable);
    }
};