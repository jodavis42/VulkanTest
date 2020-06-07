set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(ZilchScript "")

include(${CMAKE_CURRENT_LIST_DIR}/CMakeFiles.cmake)
target_sources(ZilchScript
    PRIVATE
    ${manualAdditions}
)

target_include_directories(ZilchScript
    PUBLIC
    ${CurrentDirectory}
    ${LibrariesDir}
)

ADD_MSVC_PRECOMPILED_HEADER(ZilchScript "Precompiled.hpp" "Precompiled.cpp")
Set_Common_TargetCompileOptions(ZilchScript)

target_link_libraries(ZilchScript
                      PUBLIC
                      Utilities
                      Foundation
                      Memory
                      Common
                      Zilch
                      Resources
                      Engine
)

set_target_properties(ZilchScript PROPERTIES FOLDER "Libraries")
