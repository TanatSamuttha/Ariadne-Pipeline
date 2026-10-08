#include "pipeline.hpp"

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
        Ariadne::Pipeline<State> pipeline(stateParallel);
        
        pipeline.addNode("push0", push0);
        pipeline.addNode("push1", push1);
        pipeline.addNode("push2", push2);
        pipeline.addNode("push3", push3);
        pipeline.addNode("push4", push4);
        pipeline.addNode("push5", push5);
        
        pipeline.addEdge(pipeline.START, "push0");
        pipeline.addEdge(pipeline.START, "push1");
        pipeline.addEdge(pipeline.START, "push2");
        pipeline.addEdge(pipeline.START, "push3");
        pipeline.addEdge(pipeline.START, "push4");
        pipeline.addEdge(pipeline.START, "push5");
        pipeline.addEdge("push0", pipeline.END);
        pipeline.addEdge("push1", pipeline.END);
        pipeline.addEdge("push2", pipeline.END);
        pipeline.addEdge("push3", pipeline.END);
        pipeline.addEdge("push4", pipeline.END);
        pipeline.addEdge("push5", pipeline.END);

        pipeline.spawnWorker(6);
        auto start = std::chrono::steady_clock::now();

        pipeline.exec();
        pipeline.wait();
        
        auto end = std::chrono::steady_clock::now();
        pipeline.destroyWorker();
        timeParallel = std::chrono::duration<double>(end - start).count();
    }

    auto stateSequential = std::make_shared<State>();
    double timeSequential;
    {
        Ariadne::Pipeline<State> pipeline(stateSequential);

        pipeline.addNode("pushAll", pushAll);

        pipeline.addEdge(pipeline.START, "pushAll");
        pipeline.addEdge("pushAll", pipeline.END);

        pipeline.spawnWorker(1);
        auto start = std::chrono::steady_clock::now();

        pipeline.exec();
        pipeline.wait();
        
        auto end = std::chrono::steady_clock::now();
        pipeline.destroyWorker();
        timeSequential = std::chrono::duration<double>(end - start).count();
    }

    auto stateChainSequential = std::make_shared<State>();
    double timeChainSequential;
    {
        Ariadne::Pipeline<State> pipeline(stateChainSequential);
        
        pipeline.addNode("push0", push0);
        pipeline.addNode("push1", push1);
        pipeline.addNode("push2", push2);
        pipeline.addNode("push3", push3);
        pipeline.addNode("push4", push4);
        pipeline.addNode("push5", push5);
        
        pipeline.addEdge(pipeline.START, "push0");
        pipeline.addEdge("push0", "push1");
        pipeline.addEdge("push1", "push2");
        pipeline.addEdge("push2", "push3");
        pipeline.addEdge("push3", "push4");
        pipeline.addEdge("push4", "push5");
        pipeline.addEdge("push5", pipeline.END);

        pipeline.spawnWorker(1);
        auto start = std::chrono::steady_clock::now();

        pipeline.exec();
        pipeline.wait();
        
        auto end = std::chrono::steady_clock::now();
        pipeline.destroyWorker();
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

    std::cout << "Pipeline nodes:                          " << stateParallel->n << '\n';
    std::cout << "Time parallel:                           " << timeParallel << '\n';
    std::cout << "Time sequential:                         " << timeSequential << '\n';
    std::cout << "Time chain sequential:                   " << timeChainSequential << '\n';
    std::cout << "Relative time parallel and sequential:   " << timeSequential / timeParallel << "x\n";
    std::cout << "Relative time parallel and chain:        " << timeChainSequential / timeParallel << "x\n";

    return 0;
}