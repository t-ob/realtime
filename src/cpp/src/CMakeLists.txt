#file(GLOB SOURCES "*.cpp")
#
#find_package(Boost REQUIRED)
#
#add_library(listener_lib ${SOURCES})
#target_link_libraries(listener_lib ${BOOST_LIBRARY})
#
#target_include_directories(listener_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)
#

# Creating a static library from the source files
add_library(listener_lib STATIC Session.cpp SessionManager.cpp)

# Specifying the location of the header files
target_include_directories(listener_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${Boost_INCLUDE_DIRS}
        )

# Linking against the Boost libraries
target_link_libraries(listener_lib
        ${Boost_LIBRARIES}
        )