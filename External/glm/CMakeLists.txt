set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

add_library(GLM INTERFACE)

target_include_directories(GLM 
    INTERFACE 
    ${CurrentDirectory}/glmFiles
    ${ExternalDir}
)
