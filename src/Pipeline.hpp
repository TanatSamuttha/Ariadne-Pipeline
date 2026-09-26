#pragma once

#include <functional>
#include <utility>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <map>

namespace Ariadne
{
    struct NoState {};

    template<typename T>
    class Pipeline
    {
    private:
        std::vector<std::function<std::string(std::shared_ptr<T>)>> nodes;
        std::map<std::string, size_t> nodeIds;
        std::vector<std::string> nodeNames;

        std::vector<size_t> inDegrees;
        std::vector<size_t> outDegrees;
        std::vector<size_t> remainInDegrees;

        std::vector<std::vector<size_t>> adjacent;

        std::queue<std::pair<size_t, size_t>> printQueue;
        std::queue<size_t> tasks;

        size_t concurrenting;
        bool initializedAdjacent;
        
        static std::string dummy (std::shared_ptr<T> state)
        {
            return "";
        }

        void init ()
        {
            addNode(END, dummy);
            addNode(START, dummy);
        }

    public:
        std::shared_ptr<T> state;

        static constexpr std::string START = "Start";
        static constexpr std::string END = "End";

        Pipeline () : state(nullptr)
        {
            init();
        }

        Pipeline (std::shared_ptr<T> state) : state(state)
        {
            init();
        }

        void addNode (std::string name, std::function<std::string(std::shared_ptr<T>)> callable)
        {
            nodeIds[name] = nodes.size();
            nodeNames.push_back(name);
            nodes.push_back(callable);
            inDegrees.push_back(0);
            outDegrees.push_back(0);
            remainInDegrees.push_back(0);
            initializedAdjacent = false;
        }

        void addEdge (std::string origin, std::string destination)
        {
            if (!initializedAdjacent)
            {
                adjacent = std::vector<std::vector<size_t>> (nodes.size());
                initializedAdjacent = true;
            }

            size_t originId = nodeIds[origin], destinationId = nodeIds[destination];
            adjacent[originId].push_back(destinationId);
            ++inDegrees[destinationId];
            ++outDegrees[originId];
            ++remainInDegrees[destinationId];
        }

        void exec ()
        {
            tasks.push(nodeIds[START]);
        }

        void print ()
        {
            printQueue.emplace(-1, nodeIds[START]);

            while (!printQueue.empty())
            {
                auto [parent, node] = printQueue.front();
                printQueue.pop();
                remainInDegrees[node] = inDegrees[node];

                if (parent != -1) std::cout << nodeNames[parent] << " -> " << nodeNames[node] << '\n';

                for (size_t nextNode : adjacent[node])
                {
                    --remainInDegrees[nextNode];
                    if (!remainInDegrees[nextNode])
                    {
                        printQueue.emplace(node, nextNode);
                    }
                }
            }
        }

        class WorkerPool
        {
        
        }
    };
}