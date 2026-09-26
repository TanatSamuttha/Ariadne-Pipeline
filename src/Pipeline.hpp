#pragma once

#include <functional>

template<typename T>
class Pipeline
{
public:
    T* state;

    Pipeline () = default;

    Pipeline (T* state) : state(state) {};
};