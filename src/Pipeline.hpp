#pragma once

#include <functional>
#include <string>
#include <vector>
#include <map>

template<typename T>
class Pipeline
{
private:
    std::vector<std::function<void(T*)>> node;
    std::map<std::string, size_t> nodeId;

public:
    T* state;

    Pipeline () = default;

    Pipeline (T* state) : state(state) {};

    void addNode (std::string name, std::function<void(T*)> callable)
    {
        nodeId[name] = node.size();
        node.push_back(callable);
    }
};