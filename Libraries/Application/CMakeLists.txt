set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_executable(Application "")

include(${CMAKE_CURRENT_LIST_DIR}/CMakeFiles.cmake)
target_sources(Application
    PRIVATE
    ${manualAdditions}
)

target_include_directories(Application
    PUBLIC
    ${CurrentDirectory}
    ${LibrariesDir}
)

ADD_MSVC_PRECOMPILED_HEADER(Application "Precompiled.hpp" "Precompiled.cpp")
Set_Common_TargetCompileOptions(Application)

target_link_libraries(Application
                      PUBLIC
                      Utilities
                      Graphics
                      Vulkan
                      GLFW
                      VulkanSDK
)

set_property(TARGET Application PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Application>")
add_custom_command(TARGET Application POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${ResourcesDir}/ $<TARGET_FILE_DIR:Application>)

set_target_properties(Application PROPERTIES FOLDER "Libraries")
