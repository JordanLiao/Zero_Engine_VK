
#Top level main source files
file(GLOB MAIN_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(ZeroEngineVK ${MAIN_SOURCES})

#Use Vulkan as the renderer graphics api
target_compile_definitions(ZeroEngineVK PRIVATE USE_VULKAN)

target_include_directories(ZeroEngineVK PRIVATE
    # Project Root Source Tree
    ${CMAKE_CURRENT_SOURCE_DIR}/src

    # Header-Only / SDK Dependencies
    ${DEP_DIR}/GLM
    ${DEP_DIR}/stb_image
    ${DEP_DIR}/VMA/include

    # Prebuilt Library Includes
    ${DEP_DIR}/Assimp/include
    ${DEP_DIR}/GLFW/include
    ${DEP_DIR}/spirv_cross/include
)

target_link_libraries(ZeroEngineVK PRIVATE
    Vulkan::Vulkan

    # Prebuilt static/import libraries
    "${DEP_DIR}/GLFW/lib-vc2019/glfw3.lib"
    "${DEP_DIR}/Assimp/lib/assimp-vc143-mt.lib"

    # Config-specific dependencies
    $<$<CONFIG:Debug>:${DEP_DIR}/spirv_cross/lib_debug/spirv-cross-cored.lib>
    $<$<CONFIG:Debug>:${DEP_DIR}/spirv_cross/lib_debug/spirv-cross-glsld.lib>
    $<$<NOT:$<CONFIG:Debug>>:${DEP_DIR}/spirv_cross/lib_release/spirv-cross-core.lib>
    $<$<NOT:$<CONFIG:Debug>>:${DEP_DIR}/spirv_cross/lib_release/spirv-cross-glsl.lib>
)

#set_target_properties(ZeroEngineVK PROPERTIES
#    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#)

set_target_properties(ZeroEngineVK PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/release"
)

add_custom_command(TARGET ZeroEngineVK POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${ZeroEngineVK_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:ZeroEngineVK>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:ZeroEngineVK>/shaders"
    COMMENT "Copying assets and shaders to binary directory..."
)

add_subdirectory(Graphics)
add_subdirectory(IO)
add_subdirectory(Resources)
add_subdirectory(Vulkan)
add_subdirectory(tools)