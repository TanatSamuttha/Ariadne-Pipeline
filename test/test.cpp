#include <iostream>

#include "pipeline.hpp"
#include "foo.hpp"

Ariadne::Pipeline<Ariadne::NoState> pipeline;

void Test1 ()
{
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
    pipeline.addNode("Task1", foo1);
    pipeline.addNode("Task2", foo2);
    pipeline.addNode("Task3", foo3);
    pipeline.addNode("Task4", foo4);
    pipeline.addNode("Task5", foo5);
    pipeline.addNode("Task6", foo6);
    pipeline.addNode("Task7", foo7);
    pipeline.addNode("Task8", foo8);
    pipeline.addNode("Task9", foo9);

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

int main (int argc, char* argv[])
{
    if ((std::string)argv[1] == "1") Test1();
    if ((std::string)argv[1] == "2") Test2();

    return 0;
}