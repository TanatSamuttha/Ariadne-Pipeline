#pragma once

#include <condition_variable>
#include <functional>
#include <iostream>
#include <utility>
#include <thread>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
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
        std::vector<size_t> remainInDegrees;

        std::vector<std::vector<size_t>> adjacent;

        std::queue<std::pair<size_t, size_t>> printQueue;
        std::queue<size_t> tasks;
        std::mutex tasksLock;
        std::condition_variable taskCV;
        bool working;

        bool initializedAdjacent;
        
        std::string start (std::shared_ptr<T> state)
        {
            return "";
        }

        std::string end (std::shared_ptr<T> state)
        {
            std::lock_guard lock(tasksLock);
            working = false;
            taskCV.notify_all();
            return "";
        }

        void init ()
        {
            addNode(END, std::bind(&Pipeline::end, this, std::placeholders::_1));
            addNode(START, std::bind(&Pipeline::start, this, std::placeholders::_1));
        }

        void worker ()
        {
            while (true)
            {
                std::unique_lock<std::mutex> lock(tasksLock);
                taskCV.wait(lock, [&]()
                {
                    return !tasks.empty() || !working;
                });

                if (!working)
                    return;

                size_t nodeId;
                nodeId = tasks.front();
                tasks.pop();
                
                lock.unlock();

                std::string next = nodes[nodeId](state);
                
                if (!working)
                    return;

                if (next == "")
                {
                    std::lock_guard lock(tasksLock);
                    for (size_t nextId : adjacent[nodeId])
                    {
                        pushTask(nextId);
                        taskCV.notify_one();
                    }
                }
                else
                {
                    size_t nextId = nodeIds[next];
                    std::lock_guard lock(tasksLock);
                    pushTask(nextId);
                    taskCV.notify_one();
                }
                
            }
        }

        void pushTask (size_t nextId)
        {
            --remainInDegrees[nextId];
            if (!remainInDegrees[nextId])
            {
                tasks.push(nextId);
            }
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
            ++remainInDegrees[destinationId];
        }

        void exec (size_t workers)
        {
            working = true;
            tasks.push(nodeIds[START]);

            std::vector<std::thread> threads;

            for (size_t i = 0; i < workers; ++i)
            {
                threads.emplace_back(&Pipeline::worker, this);
            }

            for (auto& thread : threads)
            {
                thread.join();
            }
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
    };
}