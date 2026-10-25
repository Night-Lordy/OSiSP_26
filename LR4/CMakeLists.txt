cmake_minimum_required(VERSION 3.10)
project(DiningPhilosophers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(DiningPhilosophers
    main.cpp
    Logger.cpp
    Philosopher.cpp
    Fork.cpp
    Stats.cpp
    Invariants.cpp
)

target_link_libraries(DiningPhilosophers PRIVATE)