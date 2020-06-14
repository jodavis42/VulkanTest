set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(Serialization "")

include(${CMAKE_CURRENT_LIST_DIR}/CMakeFiles.cmake)
target_sources(Serialization
    PRIVATE
    ${manualAdditions}
)

target_include_directories(Serialization
    PUBLIC
    ${CurrentDirectory}
    ${LibrariesDir}
)

ADD_MSVC_PRECOMPILED_HEADER(Serialization "Precompiled.hpp" "Precompiled.cpp")
Set_Common_TargetCompileOptions(Serialization)

target_link_libraries(Serialization
                      PUBLIC
                      Utilities
                      Foundation
                      Memory
                      Zilch
                      Resources
)

set_target_properties(Serialization PROPERTIES FOLDER "Libraries")
