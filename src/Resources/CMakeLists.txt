file(GLOB RESOURCES_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# Append the gathered files to the root target
target_sources(ZeroEngineVK PRIVATE
    ${RESOURCES_SOURCES}
)

target_include_directories(ZeroEngineVK PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)