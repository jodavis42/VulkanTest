set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(SPIRV-Cross  "")

include(${CMAKE_CURRENT_LIST_DIR}/CMakeFiles.cmake)
target_sources(SPIRV-Cross 
    PRIVATE
    ${manualAdditions}
)

target_include_directories(SPIRV-Cross 
    PUBLIC
    ${CurrentDirectory}
    ${LibrariesDir}
)

Set_Common_TargetCompileOptions(SPIRV-Cross )

target_link_libraries(SPIRV-Cross 
                      PUBLIC
)

set_target_properties(SPIRV-Cross  PROPERTIES FOLDER "Libraries")
