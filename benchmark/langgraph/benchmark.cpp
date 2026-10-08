#include "pipeline.hpp"
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
    Ariadne::Pipeline<State> pipeline(state);

    for (int i = 1; i <= n; ++i)
    {
        pipeline.addNode(std::to_string(i), addList);
    }

    pipeline.addEdge(pipeline.START, "1");
    for (int i = 2; i <= n; ++i)
    {
        pipeline.addEdge(std::to_string(i - 1), std::to_string(i));
    }
    pipeline.addEdge(std::to_string(n), pipeline.END);
    
    pipeline.spawnWorker(1);
    
    auto start = std::chrono::steady_clock::now();
    pipeline.exec();
    pipeline.wait();
    auto end = std::chrono::steady_clock::now();

    pipeline.destroyWorker();

    for (int i = 1; i <= n; ++i)
    {
        if (state->list[i - 1] != i)
        {
            std::cout << "List is incorrect";
            return 0;
        }
    }
    
    std::cout << "Pipeline nodes: " << n << '\n';
    std::cout << "Time:           " << std::chrono::duration<double>(end - start).count();

    return 0;
}