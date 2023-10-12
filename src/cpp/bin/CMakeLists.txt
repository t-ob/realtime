# Define the executable and specify its source files
add_executable(listener listener.cpp)

# Link against the library from src/ and the Boost libraries
target_link_libraries(listener
        PRIVATE
        listener_lib
        ${Boost_LIBRARIES}
        )

# Specify the locations of header files for the executable
target_include_directories(listener
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${Boost_INCLUDE_DIRS}
        )