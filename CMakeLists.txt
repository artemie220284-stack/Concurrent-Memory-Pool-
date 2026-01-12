cmake_minimum_required(VERSION 3.10)
project(CONCurrentMemoryPool C)

set(CMAKE_C_STANDARD 11)

add_executable(CONCurrentMemoryPool
        src/test.c
        src/UnLocked.c
        src/ThreadPool.c
        src/CentralCache.c
        src/ConcurrentAlloc.c
        src/ObjectPool.c
        src/ThreadCache.c
        src/PageCache.c
        src/MemoryManager.c)
target_include_directories(CONCurrentMemoryPool PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link against pthread and atomic libraries
target_link_libraries(CONCurrentMemoryPool pthread atomic)

# Add compiler flags for additional warnings
target_compile_options(CONCurrentMemoryPool PRIVATE -Wall -Wextra -Wpedantic)