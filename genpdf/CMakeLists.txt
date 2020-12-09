cmake_minimum_required(VERSION 3.1)

add_custom_target(Genpdf ALL
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND cargo build --release
    COMMENT "Building Rust library genpdf..."
)
