target_sources(SPIRV-Cross
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/GLSL.std.450.h
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv.h
    ${CMAKE_CURRENT_LIST_DIR}/spirv.hpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cfg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cfg.hpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_common.hpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cpp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cpp.hpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cross.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cross.hpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cross_c.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cross_c.h
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cross_containers.hpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cross_error_handling.hpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cross_parsed_ir.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cross_parsed_ir.hpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cross_util.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_cross_util.hpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_glsl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_glsl.hpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_hlsl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_hlsl.hpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_msl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_msl.hpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_parser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_parser.hpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_reflect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spirv_reflect.hpp
)