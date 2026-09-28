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
        struct Incedent
        {
            size_t nodeId;
            bool edgeType; // 1 = cycle

            Incedent () = default;
            Incedent (size_t nodeId, bool edgeType) : nodeId(nodeId), edgeType(edgeType) {}
        };

        struct Task
        {
            size_t nodeId;
            bool active;
            
            Task () = default;
            Task (size_t nodeId, bool active) : nodeId(nodeId), active(active) {}
        };
        
        struct Printable
        {
            size_t nodeId;
            size_t parentId;
            bool edgeType;
            
            Printable () = default;
            Printable (size_t nodeId, size_t parentId, bool edgeType) : nodeId(nodeId), parentId(parentId), edgeType(edgeType) {}
        };

        std::vector<std::function<std::string(std::shared_ptr<T>)>> nodes;
        std::map<std::string, size_t> nodeIds;
        std::vector<std::string> nodeNames;

        std::vector<size_t> inDegrees;
        std::vector<size_t> remainInDegrees;

        std::vector<std::vector<Incedent>> adjacent;

        std::queue<Printable>printQueue;
        std::queue<Task> tasks;
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
                std::unique_lock<std::mutex> uLock(tasksLock);
                taskCV.wait(uLock, [&]()
                {
                    return !tasks.empty() || !working;
                });

                if (!working)
                    return;

                Task task = tasks.front();
                tasks.pop();
                
                uLock.unlock();

                std::string next = "";
                if(task.active) next = nodes[task.nodeId](state);
                
                if (!working)
                    return;

                size_t nextId = nodeIds[next];
                std::lock_guard gLock(tasksLock);
                for (Incedent nextTask : adjacent[task.nodeId])
                {
                    if (next != "" && nextId == nextTask.nodeId) pushTask(nextTask.nodeId, true, nextTask.edgeType);
                    else pushTask(nextTask.nodeId, task.active, nextTask.edgeType);
                    taskCV.notify_one();
                }
                remainInDegrees[task.nodeId] = inDegrees[task.nodeId];
            }
        }

        void pushTask (size_t nextId, bool active, bool edgeType)
        {
            if (edgeType) tasks.emplace(nextId, active);
            else
            {
                --remainInDegrees[nextId];
                if (!remainInDegrees[nextId]) tasks.emplace(nextId, active);
            }
            
        }

        void generalAddEdge (std::string origin, std::string destination, bool edgeType)
        {
            if (!initializedAdjacent)
            {
                adjacent = std::vector<std::vector<Incedent>> (nodes.size());
                initializedAdjacent = true;
            }

            size_t originId = nodeIds[origin], destinationId = nodeIds[destination];
            adjacent[originId].emplace_back(destinationId, edgeType);
            if (!edgeType)
            {
                ++inDegrees[destinationId];
                ++remainInDegrees[destinationId];
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
            generalAddEdge(origin, destination, false);
        }

        void addCycleEdge (std::string origin, std::string destination)
        {
            generalAddEdge(origin, destination, true);
        }

        void exec (size_t workers)
        {
            working = true;
            pushTask(nodeIds[START], true, true);

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
            printQueue.emplace(nodeIds[START], -1, false);

            while (!printQueue.empty())
            {
                Printable node = printQueue.front();
                printQueue.pop();
                remainInDegrees[node.nodeId] = inDegrees[node.nodeId];

                if (node.parentId != -1) std::cout << nodeNames[node.parentId] << " -> " << nodeNames[node.nodeId] << '\n';

                for (Incedent nextNode : adjacent[node.nodeId])
                {
                    if (nextNode.edgeType)
                    {
                        std::cout << nodeNames[node.nodeId] << " C^ " << nodeNames[nextNode.nodeId] << '\n';
                        continue;
                    }
                    --remainInDegrees[nextNode.nodeId];
                    if (!remainInDegrees[nextNode.nodeId])
                    {
                        printQueue.emplace(nextNode.nodeId, node.nodeId, nextNode.edgeType);
                    }
                }
            }
        }
    };
}