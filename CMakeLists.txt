cmake_minimum_required(VERSION 3.10)
project(prototype2)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(SQLite3 REQUIRED)
find_package(LLVM REQUIRED CONFIG)

# Get LLVM configuration
llvm_map_components_to_libnames(llvm_libs
    orcjit
    native
    mcjit
)

# Create library targets for each module
add_library(core
    src/core/run.cpp
    src/core/decision_maker.cpp
)

add_library(utils
    src/utils/measure_time.cpp
    src/utils/file_management.cpp
    src/utils/utils.cpp
    src/utils/randomness_utils.cpp
)

add_library(mutations
    src/mutations/mutation_factory.cpp
    src/mutations/add_random_arithmetic.cpp
    src/mutations/add_new_cond.cpp
    src/mutations/mutation.cpp
    src/mutations/move_blockwise.cpp
    src/mutations/replace_arithmetic.cpp
)

add_library(tuning_strategies
    src/tuning_strategies/random_tuning.cpp
    src/tuning_strategies/genetic_tuning.cpp
)

# Add the main executable
add_executable(main src/main.cpp)

# Set include directories for all targets
target_include_directories(core 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/src
        ${LLVM_INCLUDE_DIRS}
)
target_include_directories(utils 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/src
        ${LLVM_INCLUDE_DIRS}
)
target_include_directories(mutations 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/src
        ${LLVM_INCLUDE_DIRS}
)
target_include_directories(tuning_strategies 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/src
        ${LLVM_INCLUDE_DIRS}
)
target_include_directories(main 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/src
        ${LLVM_INCLUDE_DIRS}
)

# Add LLVM definitions to all targets
target_compile_definitions(core PRIVATE ${LLVM_DEFINITIONS})
target_compile_definitions(utils PRIVATE ${LLVM_DEFINITIONS})
target_compile_definitions(mutations PRIVATE ${LLVM_DEFINITIONS})
target_compile_definitions(tuning_strategies PRIVATE ${LLVM_DEFINITIONS})
target_compile_definitions(main PRIVATE ${LLVM_DEFINITIONS})

# Link libraries
target_link_libraries(core
    PRIVATE
    ${SQLite3_LIBRARIES}
    ${llvm_libs}
)

target_link_libraries(utils
    PRIVATE
    ${SQLite3_LIBRARIES}
    ${llvm_libs}
)

target_link_libraries(mutations
    PRIVATE
    ${SQLite3_LIBRARIES}
    ${llvm_libs}
)

target_link_libraries(tuning_strategies
    PRIVATE
    ${SQLite3_LIBRARIES}
    ${llvm_libs}
)

target_link_libraries(main
    PRIVATE
    core
    utils
    mutations
    tuning_strategies
    ${SQLite3_LIBRARIES}
    ${llvm_libs}
)



# # Copy necessary files to build directory
# add_custom_command(TARGET main POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         ${CMAKE_SOURCE_DIR}/original.ll
#         ${CMAKE_SOURCE_DIR}/results.db
#         ${CMAKE_SOURCE_DIR}/genetic_results.csv
#         ${CMAKE_SOURCE_DIR}/results.csv
#         ${CMAKE_SOURCE_DIR}/10k.csv
#         $<TARGET_FILE_DIR:main>
# )

# Add a custom target to run the program
add_custom_target(run
    COMMAND $<TARGET_FILE:main>
    DEPENDS main
    WORKING_DIRECTORY $<TARGET_FILE_DIR:main>
)