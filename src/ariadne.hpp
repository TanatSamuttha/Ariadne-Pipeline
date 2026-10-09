#pragma once

#include <condition_variable>
#include <functional>
#include <stdexcept>
#include <algorithm>
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
    class Graph
    {
    private:
        struct Incedent
        {
            size_t nodeId;
            bool isCycle;

            Incedent () = default;
            Incedent (size_t nodeId, bool isCycle) : nodeId(nodeId), isCycle(isCycle) {}

            bool operator== (const Incedent rhs) const
            {
                return nodeId == rhs.nodeId;
            }

            bool operator< (const Incedent rhs) const
            {
                return nodeId < rhs.nodeId;
            }
        };

        struct Task
        {
            size_t nodeId;
            
            Task () = default;
            Task (size_t nodeId) : nodeId(nodeId) {}
        };
        
        struct Printable
        {
            size_t nodeId;
            size_t parentId;
            bool isCycle;
            
            Printable () = default;
            Printable (size_t nodeId, size_t parentId, bool isCycle) : nodeId(nodeId), parentId(parentId), isCycle(isCycle) {}
        };

        std::vector<std::function<std::string(std::shared_ptr<T>)>> nodes;
        std::map<std::string, size_t> nodeIds;
        std::vector<std::string> nodeNames;

        std::vector<uint64_t> isActive;
        std::mutex activeLock;

        std::vector<size_t> inDegrees;
        std::vector<size_t> remainInDegrees;

        std::vector<std::vector<Incedent>> adjacent;

        std::vector<std::thread> threads;

        std::queue<Printable> printQueue;
        std::queue<Task> tasks;
        std::mutex tasksLock;
        std::condition_variable taskCV;
        bool working;

        std::mutex executingLock;
        std::condition_variable executingCV;
        bool executing;

        bool initializedAdjacent;

        void setActive (size_t idx, bool value)
        {
            uint64_t mask = 1ull << (idx % 64);

            std::lock_guard<std::mutex> gLock(activeLock);
            if (value) isActive[idx / 64] |= mask;
            else       isActive[idx / 64] &= ~mask;
        }

        bool readActive (size_t idx)
        {
            uint64_t mask = 1ull << (idx % 64);
            std::lock_guard<std::mutex> gLock(activeLock);
            return (isActive[idx / 64] & mask) != 0;
        }
        
        std::string start (std::shared_ptr<T> state)
        {
            return "";
        }

        std::string end (std::shared_ptr<T> state)
        {
            std::lock_guard<std::mutex> gLock(executingLock);
            executing = false;
            executingCV.notify_one();
            return "";
        }

        void init ()
        {
            addNode(END, std::bind(&Graph::end, this, std::placeholders::_1));
            addNode(START, std::bind(&Graph::start, this, std::placeholders::_1));
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

                if (readActive(task.nodeId)) next = nodes[task.nodeId](state);
                
                if (!working)
                    return;

                std::lock_guard gTaskLock(tasksLock);
                    
                if (next != "")
                {
                    size_t nextId = nodeIds[next];
                    typename std::vector<Incedent>::iterator it;
                    if (adjacent[task.nodeId].size() < 100)
                        it = std::find(adjacent[task.nodeId].begin(), adjacent[task.nodeId].end(), Incedent(nextId, false));
                    else
                        it = std::lower_bound(adjacent[task.nodeId].begin(), adjacent[task.nodeId].end(), Incedent(nextId, false));
                    if (it->isCycle)
                    {
                        pushTask(it->nodeId, true, true);
                    }
                    else
                    {
                        for (Incedent nextTask : adjacent[task.nodeId])
                        {
                            if (!nextTask.isCycle) pushTask(nextTask.nodeId, !(nextTask.nodeId ^ nextId), false);
                        }
                    }
                }
                else
                {
                    for (Incedent nextTask : adjacent[task.nodeId])
                    {
                        if (!nextTask.isCycle) pushTask(nextTask.nodeId, readActive(task.nodeId), false);
                    }
                }

                remainInDegrees[task.nodeId] = inDegrees[task.nodeId];
                setActive(task.nodeId, false);
            }
        }

        void pushTask (size_t nextId, bool active, bool isCycle)
        {
            if (isCycle)
            { 
                tasks.emplace(nextId);
                setActive(nextId, active);
            }
            else
            {
                --remainInDegrees[nextId];
                if (!readActive(nextId))
                    setActive(nextId, active);

                if (!remainInDegrees[nextId])
                    tasks.emplace(nextId);
            }
            taskCV.notify_one();
        }

        void generalAddEdge (std::string origin, std::string destination, bool isCycle)
        {
            if (!initializedAdjacent)
            {
                adjacent = std::vector<std::vector<Incedent>> (nodes.size());
                isActive.assign(nodes.size() / 64 + (nodes.size() % 64 != 0), 0);
                initializedAdjacent = true;
            }

            size_t originId = nodeIds[origin], destinationId = nodeIds[destination];
            adjacent[originId].emplace_back(destinationId, isCycle);
            if (!isCycle)
            {
                ++inDegrees[destinationId];
                ++remainInDegrees[destinationId];
            }
        }

    public:
        std::shared_ptr<T> state;

        static constexpr std::string START = "Start";
        static constexpr std::string END = "End";

        Graph () : state(nullptr)
        {
            init();
        }

        Graph (std::shared_ptr<T> state) : state(state)
        {
            init();
        }

        Graph (const Graph& other)
        {
            nodes = other.nodes;
            nodeIds = other.nodeIds;
            nodeNames = other.nodeNames;
            inDegrees = other.inDegrees;
            remainInDegrees = other.inDegrees;
            adjacent = other.adjacent;
            initializedAdjacent = true;
        }

        Graph& operator= (const Graph& other)
        {
            nodes = other.nodes;
            nodeIds = other.nodeIds;
            nodeNames = other.nodeNames;
            inDegrees = other.inDegrees;
            remainInDegrees = other.inDegrees;
            adjacent = other.adjacent;
            initializedAdjacent = true;

            return *this;
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

        void spawnWorker (size_t workers)
        {
            working = true;
            for (size_t i = 0; i < workers; ++i)
            {
                threads.emplace_back(&Graph::worker, this);
            }
        }

        void destroyWorker ()
        {
            tasksLock.lock();
            working = false;
            tasksLock.unlock();
            taskCV.notify_all();
            for (auto& thread : threads)
            {
                thread.join();
            }
            threads.clear();
        }

        void exec ()
        {
            if (!working)
                throw std::runtime_error("No worker");
            executing = true;
            pushTask(nodeIds[START], true, true);
        }

        void wait ()
        {
            std::unique_lock<std::mutex> uLock(executingLock);
            executingCV.wait(uLock, [&]()
            {
                return !executing;
            });
        }

        void print ()
        {
            printQueue.emplace(nodeIds[START], -1, false);

            while (!printQueue.empty())
            {
                Printable node = printQueue.front();
                printQueue.pop();
                remainInDegrees[node.nodeId] = inDegrees[node.nodeId];

                for (Incedent nextNode : adjacent[node.nodeId])
                {
                    if (nextNode.isCycle)
                    {
                        std::cout << nodeNames[node.nodeId] << " C^ " << nodeNames[nextNode.nodeId] << '\n';
                        continue;
                    }
                    else std::cout << nodeNames[node.nodeId] << " -> " << nodeNames[nextNode.nodeId] << '\n';
                    --remainInDegrees[nextNode.nodeId];
                    if (!remainInDegrees[nextNode.nodeId])
                    {
                        printQueue.emplace(nextNode.nodeId, node.nodeId, nextNode.isCycle);
                    }
                }
            }
        }
    };
}