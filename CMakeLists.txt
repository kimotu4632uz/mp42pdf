cmake_minimum_required(VERSION 3.1)

project(mp42pdf CXX)

add_subdirectory(genpdf)

find_package(OpenCV REQUIRED COMPONENTS core imgproc video videoio)

include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(mp42pdf main.cpp)
add_dependencies(mp42pdf Genpdf)

target_compile_features(mp42pdf PUBLIC cxx_std_11)
target_link_libraries(mp42pdf -lpthread -ldl ${mp42pdf_SOURCE_DIR}/genpdf/target/release/libgenpdf.a ${OpenCV_LIBS})
