#include <iostream>

#include "pipeline.hpp"

Pipeline<std::monostate> pipeline;

void Test1 ()
{
    std::function<std::string(std::monostate*)> dummy = [](std::monostate*) -> std::string
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

    pipeline.print();
}

int main (int argc, char* argv[])
{
    if ((std::string)argv[1] == "1") Test1();

    return 0;
}