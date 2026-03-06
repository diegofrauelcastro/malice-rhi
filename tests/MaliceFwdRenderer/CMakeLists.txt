
# Test project name
project(MaliceFWDRenderer)

# Look for header and source files.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "*.h")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(MaliceRHI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/code/include)

target_link_libraries(${PROJECT_NAME} PRIVATE MaliceRHI)

add_custom_target(copy_data
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different 
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
    COMMENT "Copying data"
)
add_dependencies(${PROJECT_NAME} copy_data)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})