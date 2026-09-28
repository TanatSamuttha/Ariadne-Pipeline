#include <iostream>

#include "pipeline.hpp"
#include "foo.hpp"

struct State
{
    uint64_t sum = 0;
};

void Test1 ()
{
    Ariadne::Pipeline<Ariadne::NoState> pipeline;

    std::function<std::string(std::shared_ptr<Ariadne::NoState>)> dummy = [](std::shared_ptr<Ariadne::NoState>) -> std::string
    {
        return "";
    };

    pipeline.addNode("Task1", dummy);
    pipeline.addNode("Task2", dummy);
    pipeline.addNode("Task3", dummy);
    pipeline.addNode("Task4", dummy);

    
    pipeline.addEdge(pipeline.START, "Task1");
    pipeline.addEdge("Task1", "Task2");
    pipeline.addEdge("Task2", "Task3");
    pipeline.addEdge("Task2", "Task4");
    pipeline.addEdge("Task4", pipeline.END);
    std::cout << "Edge\n";

    pipeline.print();
}

void Test2 ()
{
    Ariadne::Pipeline<Ariadne::NoState> pipeline;

    pipeline.addNode("Task1", foo1<Ariadne::NoState>);
    pipeline.addNode("Task2", foo2<Ariadne::NoState>);
    pipeline.addNode("Task3", foo3<Ariadne::NoState>);
    pipeline.addNode("Task4", foo4<Ariadne::NoState>);
    pipeline.addNode("Task5", foo5<Ariadne::NoState>);
    pipeline.addNode("Task6", foo6<Ariadne::NoState>);
    pipeline.addNode("Task7", foo7<Ariadne::NoState>);
    pipeline.addNode("Task8", foo8<Ariadne::NoState>);
    pipeline.addNode("Task9", foo9<Ariadne::NoState>);

    pipeline.addEdge(pipeline.START, "Task1");
    pipeline.addEdge(pipeline.START, "Task2");
    pipeline.addEdge("Task1", "Task4");
    pipeline.addEdge("Task4", "Task6");
    pipeline.addEdge("Task1", "Task3");
    pipeline.addEdge("Task2", "Task5");
    pipeline.addEdge("Task5", "Task6");
    pipeline.addEdge("Task3", pipeline.END);
    pipeline.addEdge("Task6", pipeline.END);

    pipeline.print();
    pipeline.exec(3);
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
    
    Ariadne::Pipeline<State> pipeline(std::make_shared<State>(state));

    pipeline.addNode("1", foo1<State>);
    pipeline.addNode("2", foo2<State>);
    pipeline.addNode("3", foo3<State>);
    pipeline.addNode("4", foo4<State>);
    pipeline.addNode("5", con5);
    pipeline.addNode("6", foo6<State>);
    pipeline.addNode("7", foo7<State>);
    pipeline.addNode("8", foo8<State>);
    pipeline.addNode("sum", sum);

    pipeline.addEdge(pipeline.START, "1");
    pipeline.addEdge(pipeline.START, "2");
    pipeline.addEdge("1", "sum");
    pipeline.addEdge("2", "3");
    pipeline.addEdge("sum", "6");
    pipeline.addEdge("sum", "5");
    pipeline.addCycleEdge("sum", "1");
    pipeline.addEdge("3", "6");
    pipeline.addEdge("5", "7");
    pipeline.addEdge("5", "8");
    pipeline.addEdge("6", pipeline.END);
    pipeline.addEdge("7", pipeline.END);
    pipeline.addEdge("8", pipeline.END);

    pipeline.print();
    pipeline.exec(3);
}

int main (int argc, char* argv[])
{
    if ((std::string)argv[1] == "1") Test1();
    if ((std::string)argv[1] == "2") Test2();
    if ((std::string)argv[1] == "3") Test3();

    return 0;
}