#include <iostream>

#include "ariadne.hpp"
#include "foo.hpp"

struct State
{
    uint64_t sum = 0;
};

void Test1 ()
{
    Ariadne::Graph<Ariadne::NoState> graph;

    std::function<std::string(std::shared_ptr<Ariadne::NoState>)> dummy = [](std::shared_ptr<Ariadne::NoState>) -> std::string
    {
        return "";
    };

    graph.addNode("Task1", dummy);
    graph.addNode("Task2", dummy);
    graph.addNode("Task3", dummy);
    graph.addNode("Task4", dummy);

    
    graph.addEdge(graph.START, "Task1");
    graph.addEdge("Task1", "Task2");
    graph.addEdge("Task2", "Task3");
    graph.addEdge("Task2", "Task4");
    graph.addEdge("Task4", graph.END);
    std::cout << "Edge\n";

    graph.print();
}

void Test2 ()
{
    Ariadne::Graph<Ariadne::NoState> graph;

    graph.addNode("Task1", foo1<Ariadne::NoState>);
    graph.addNode("Task2", foo2<Ariadne::NoState>);
    graph.addNode("Task3", foo3<Ariadne::NoState>);
    graph.addNode("Task4", foo4<Ariadne::NoState>);
    graph.addNode("Task5", foo5<Ariadne::NoState>);
    graph.addNode("Task6", foo6<Ariadne::NoState>);
    graph.addNode("Task7", foo7<Ariadne::NoState>);
    graph.addNode("Task8", foo8<Ariadne::NoState>);
    graph.addNode("Task9", foo9<Ariadne::NoState>);

    graph.addEdge(graph.START, "Task1");
    graph.addEdge(graph.START, "Task2");
    graph.addEdge("Task1", "Task4");
    graph.addEdge("Task4", "Task6");
    graph.addEdge("Task1", "Task3");
    graph.addEdge("Task2", "Task5");
    graph.addEdge("Task5", "Task6");
    graph.addEdge("Task3", graph.END);
    graph.addEdge("Task6", graph.END);

    graph.print();
    
    graph.spawnWorker(3);
    graph.exec();
    graph.wait();
    graph.destroyWorker();
}

void Test3 ()
{
    std::function<std::string(std::shared_ptr<State>)> sum = [](std::shared_ptr<State> state) -> std::string
    {
        std::string output = "Sum: " + std::to_string(state->sum) + "\n";
        std::cout << output;
        ++state->sum;
        if (state->sum < 10) return "1";
        else return "5";
    };

    std::function<std::string(std::shared_ptr<State>)> con5 = [](std::shared_ptr<State> state) -> std::string
    {
        for (int i = 1; i < 7; ++i)
        {
            std::cout << "con5\n";
        }
        return "8";
    };

    State state;
    
    Ariadne::Graph<State> graph(std::make_shared<State>(state));

    graph.addNode("1", foo1<State>);
    graph.addNode("2", foo2<State>);
    graph.addNode("3", foo3<State>);
    graph.addNode("4", foo4<State>);
    graph.addNode("5", con5);
    graph.addNode("6", foo6<State>);
    graph.addNode("7", foo7<State>);
    graph.addNode("8", foo8<State>);
    graph.addNode("sum", sum);

    graph.addEdge(graph.START, "1");
    graph.addEdge(graph.START, "2");
    graph.addEdge("1", "sum");
    graph.addEdge("2", "3");
    graph.addEdge("sum", "6");
    graph.addEdge("sum", "5");
    graph.addCycleEdge("sum", "1");
    graph.addEdge("3", "6");
    graph.addEdge("5", "7");
    graph.addEdge("5", "8");
    graph.addEdge("6", graph.END);
    graph.addEdge("7", graph.END);
    graph.addEdge("8", graph.END);

    graph.print();

    graph.spawnWorker(3);
    graph.exec();
    graph.wait();
    graph.destroyWorker();
}

int main (int argc, char* argv[])
{
    if ((std::string)argv[1] == "1") Test1();
    if ((std::string)argv[1] == "2") Test2();
    if ((std::string)argv[1] == "3") Test3();

    return 0;
}