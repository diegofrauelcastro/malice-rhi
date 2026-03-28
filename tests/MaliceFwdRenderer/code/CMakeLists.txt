
# Test project name
project(MaliceFWDRenderer)

# Look for header and source files.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "include/*.h")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
#target_include_directories(MaliceRHI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/code/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/imgui-docking)

# Link the RHI lib and glm for math.
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    MaliceRHI
    glm::glm
    imgui::imgui
    volk
    glfw
)

# Copy "resources/" folder.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
    COMMENT "Copying resources"
)

# Mark this project as a test project of the MaliceRHI lib.
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})