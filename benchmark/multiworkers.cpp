#include "ariadne.hpp"

struct State
{
    int n = 1000;
    std::vector<long long> v[6];
};

long long factorial (int x)
{
    long long fac = 1;
    for (long long i = 2; i <= 100000; ++i)
    {
        fac *= i;
        fac %= ((!x || !(x%25))? 25 : x%25);
    }
    return fac;
}

std::string push0 (std::shared_ptr<State> state)
{
    for (int i = 0; i < state->n; ++i)
    {
        state->v[0].push_back(factorial(i));
    }
    return "";
}

std::string push1 (std::shared_ptr<State> state)
{
    for (int i = 0; i < state->n; ++i)
    {
        state->v[1].push_back(factorial(i));
    }
    return "";
}

std::string push2 (std::shared_ptr<State> state)
{
    for (int i = 0; i < state->n; ++i)
    {
        state->v[2].push_back(factorial(i));
    }
    return "";
}

std::string push3 (std::shared_ptr<State> state)
{
    for (int i = 0; i < state->n; ++i)
    {
        state->v[3].push_back(factorial(i));
    }
    return "";
}

std::string push4 (std::shared_ptr<State> state)
{
    for (int i = 0; i < state->n; ++i)
    {
        state->v[4].push_back(factorial(i));
    }
    return "";
}

std::string push5 (std::shared_ptr<State> state)
{
    for (int i = 0; i < state->n; ++i)
    {
        state->v[5].push_back(factorial(i));
    }
    return "";
}

std::string pushAll (std::shared_ptr<State> state)
{
    for (int i = 0; i < state->n; ++i)
    {
        state->v[0].push_back(factorial(i));
        state->v[1].push_back(factorial(i));
        state->v[2].push_back(factorial(i));
        state->v[3].push_back(factorial(i));
        state->v[4].push_back(factorial(i));
        state->v[5].push_back(factorial(i));
    }
    return "";
}

int main ()
{
    auto stateParallel = std::make_shared<State>();
    double timeParallel;
    {
        Ariadne::Graph<State> graph(stateParallel);
        
        graph.addNode("push0", push0);
        graph.addNode("push1", push1);
        graph.addNode("push2", push2);
        graph.addNode("push3", push3);
        graph.addNode("push4", push4);
        graph.addNode("push5", push5);
        
        graph.addEdge(graph.START, "push0");
        graph.addEdge(graph.START, "push1");
        graph.addEdge(graph.START, "push2");
        graph.addEdge(graph.START, "push3");
        graph.addEdge(graph.START, "push4");
        graph.addEdge(graph.START, "push5");
        graph.addEdge("push0", graph.END);
        graph.addEdge("push1", graph.END);
        graph.addEdge("push2", graph.END);
        graph.addEdge("push3", graph.END);
        graph.addEdge("push4", graph.END);
        graph.addEdge("push5", graph.END);

        graph.spawnWorker(6);
        auto start = std::chrono::steady_clock::now();

        graph.exec();
        graph.wait();
        
        auto end = std::chrono::steady_clock::now();
        graph.destroyWorker();
        timeParallel = std::chrono::duration<double>(end - start).count();
    }

    auto stateSequential = std::make_shared<State>();
    double timeSequential;
    {
        Ariadne::Graph<State> graph(stateSequential);

        graph.addNode("pushAll", pushAll);

        graph.addEdge(graph.START, "pushAll");
        graph.addEdge("pushAll", graph.END);

        graph.spawnWorker(1);
        auto start = std::chrono::steady_clock::now();

        graph.exec();
        graph.wait();
        
        auto end = std::chrono::steady_clock::now();
        graph.destroyWorker();
        timeSequential = std::chrono::duration<double>(end - start).count();
    }

    auto stateChainSequential = std::make_shared<State>();
    double timeChainSequential;
    {
        Ariadne::Graph<State> graph(stateChainSequential);
        
        graph.addNode("push0", push0);
        graph.addNode("push1", push1);
        graph.addNode("push2", push2);
        graph.addNode("push3", push3);
        graph.addNode("push4", push4);
        graph.addNode("push5", push5);
        
        graph.addEdge(graph.START, "push0");
        graph.addEdge("push0", "push1");
        graph.addEdge("push1", "push2");
        graph.addEdge("push2", "push3");
        graph.addEdge("push3", "push4");
        graph.addEdge("push4", "push5");
        graph.addEdge("push5", graph.END);

        graph.spawnWorker(1);
        auto start = std::chrono::steady_clock::now();

        graph.exec();
        graph.wait();
        
        auto end = std::chrono::steady_clock::now();
        graph.destroyWorker();
        timeChainSequential = std::chrono::duration<double>(end - start).count();
    }

    if (stateParallel->n != stateSequential->n)
    {
        std::cout << "List is incorrect";
        return 0;
    }
    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < stateParallel->n; ++j)
        {
            if 
            (
                stateParallel->v[i][j] != stateSequential->v[i][j] ||
                stateParallel->v[i][j] != stateChainSequential->v[i][j]
            )
            {
                std::cout << "List is incorrect";
                return 0;
            }
        }
    }

    std::cout << "graph nodes:                          " << stateParallel->n << '\n';
    std::cout << "Time parallel:                           " << timeParallel << '\n';
    std::cout << "Time sequential:                         " << timeSequential << '\n';
    std::cout << "Time chain sequential:                   " << timeChainSequential << '\n';
    std::cout << "Relative time parallel and sequential:   " << timeSequential / timeParallel << "x\n";
    std::cout << "Relative time parallel and chain:        " << timeChainSequential / timeParallel << "x\n";

    return 0;
}