#include "ariadne.hpp"
#include <chrono>

struct State
{
    int i;
    std::vector<int> list;
};

std::string addList (std::shared_ptr<State> state)
{
    state->list.push_back(++state->i);
    return "";
}

int main ()
{
    int n = 25;

    auto state = std::make_shared<State>();
    Ariadne::Graph<State> graph(state);

    for (int i = 1; i <= n; ++i)
    {
        graph.addNode(std::to_string(i), addList);
    }

    graph.addEdge(graph.START, "1");
    for (int i = 2; i <= n; ++i)
    {
        graph.addEdge(std::to_string(i - 1), std::to_string(i));
    }
    graph.addEdge(std::to_string(n), graph.END);
    
    graph.spawnWorker(1);
    
    auto start = std::chrono::steady_clock::now();
    graph.exec();
    graph.wait();
    auto end = std::chrono::steady_clock::now();

    graph.destroyWorker();

    for (int i = 1; i <= n; ++i)
    {
        if (state->list[i - 1] != i)
        {
            std::cout << "List is incorrect";
            return 0;
        }
    }
    
    std::cout << "graph nodes: " << n << '\n';
    std::cout << "Time:           " << std::chrono::duration<double>(end - start).count();

    return 0;
}