file(GLOB_RECURSE TOOLS_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

target_sources(ZeroEngineVK PRIVATE
    ${TOOLS_SOURCES}
)

target_include_directories(ZeroEngineVK PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)