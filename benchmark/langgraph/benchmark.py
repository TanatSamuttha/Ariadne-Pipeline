from typing import TypedDict

from langgraph.graph import StateGraph, START, END
import time


class State(TypedDict):
    i: int
    list: list[int]


def add_list(state: State):
    new_i = state["i"] + 1
    state["list"].append(new_i)

    return {
        "i": new_i,
        "list": state["list"],
    }


def main():
    n = 25

    builder = StateGraph(State)

    for i in range(1, n + 1):
        builder.add_node(str(i), add_list)

    builder.add_edge(START, "1")

    for i in range(2, n + 1):
        builder.add_edge(str(i - 1), str(i))

    builder.add_edge(str(n), END)

    graph = builder.compile()

    initial_state: State = {
        "i": 0,
        "list": [],
    }

    start = time.perf_counter()

    result = graph.invoke(
        initial_state,
        config={
            "recursion_limit": n + 10
        }
    )

    end = time.perf_counter()

    for i in range(1, n + 1):
        if result["list"][i - 1] != i:
            print("List is incorrect")
            return

    print(f"Pipeline nodes: {n}")
    print(f"Time:           {end - start:.6f}")


if __name__ == "__main__":
    main()