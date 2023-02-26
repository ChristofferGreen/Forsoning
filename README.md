# Introduction
Forsoning is a header-only C++20 library implementing a coordination language similar to a JSON structure. It supports adding many built in types, serializable structs or lambdas/coroutines.

# Example Code #1
Create a space that can be filled with path/value pairs. Insert a value, then read it and grab it. 
```
PathSpaceTE space = PathSpace{};
space.insert("/hello_world", 5);
std::optional<int> value1 = space.read<int>("/hello_world");
std::optional<int> value2 = space.grab<int>("/hello_world");
```

# Example Code #2
Create a space that can be filled with path/value pairs. Insert a coroutine that returns a value and wait for that value to arrive.
```
PathSpaceTE space = PathSpace{};
space.insert("/coro", []() -> Coroutine {co_return 5;});
int value = space.grabBlock<int>("/coro");
```

# 
