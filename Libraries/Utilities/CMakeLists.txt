set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(Utilities "")

include(${CMAKE_CURRENT_LIST_DIR}/CMakeFiles.cmake)
target_sources(Utilities
    PRIVATE
    ${manualAdditions}
)

target_include_directories(Utilities
    PUBLIC
    ${CurrentDirectory}
    ${LibrariesDir}
)

ADD_MSVC_PRECOMPILED_HEADER(Utilities "Precompiled.hpp" "Precompiled.cpp")
Set_Common_TargetCompileOptions(Utilities)

target_link_libraries(Utilities
                      PUBLIC
                      RapidJson
                      ZilchShaders
)

set_target_properties(Utilities PROPERTIES FOLDER "Libraries")
