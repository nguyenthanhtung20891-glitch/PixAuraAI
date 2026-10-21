cmake_minimum_required(VERSION 3.22)
project(PixAuraAndroid LANGUAGES C CXX)
set(BUILD_TESTING OFF CACHE BOOL "Host tests not executable on Android" FORCE)
add_subdirectory(../../../ core)
option(PIXAURA_CERTIFICATION_TESTING "Test-only replay corpus in debug app" OFF)
if(PIXAURA_CERTIFICATION_TESTING)
    target_compile_definitions(pixaura_core PRIVATE PIXAURA_CERTIFICATION_TESTING PIXAURA_PREVIEW_TESTING)
    target_compile_definitions(pixaura_storage PRIVATE PIXAURA_CERTIFICATION_TESTING)
endif()
if(CMAKE_HOST_WIN32)
    set(shader_host windows-x86_64)
elseif(CMAKE_HOST_APPLE)
    set(shader_host darwin-x86_64)
else()
    set(shader_host linux-x86_64)
endif()
find_program(PIXAURA_GLSLC NAMES glslc glslc.exe PATHS "${ANDROID_NDK}/shader-tools/${shader_host}" NO_DEFAULT_PATH REQUIRED)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/exposure_spirv.h"
    COMMAND "${PIXAURA_GLSLC}" --target-env=vulkan1.0 -O0 "${CMAKE_CURRENT_SOURCE_DIR}/exposure.comp" -o "${CMAKE_CURRENT_BINARY_DIR}/exposure.spv"
    COMMAND "${CMAKE_COMMAND}" "-DINPUT=${CMAKE_CURRENT_BINARY_DIR}/exposure.spv" "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/exposure_spirv.h" -P "${CMAKE_CURRENT_SOURCE_DIR}/../../../scripts/embed-spirv.cmake"
    DEPENDS exposure.comp ../../../scripts/embed-spirv.cmake VERBATIM)
add_library(pixaura_bridge SHARED bridge.cpp editor_bridge.cpp agent_bridge.cpp asset_publication_bridge.cpp capability_bridge.cpp gpu_vulkan.cpp "${CMAKE_CURRENT_BINARY_DIR}/exposure_spirv.h")
if(PIXAURA_CERTIFICATION_TESTING)
    target_sources(pixaura_bridge PRIVATE cross_tier_certification_bridge.cpp ../../../packages/core/tests/certification/certification.cpp)
    target_compile_definitions(pixaura_bridge PRIVATE PIXAURA_CERTIFICATION_TESTING)
endif()
target_include_directories(pixaura_bridge PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(pixaura_bridge PRIVATE pixaura_core pixaura_storage vulkan)
pixaura_strict(pixaura_bridge)

# The shared inference authority owns the Android backend; JNI only invokes it.
find_program(PIXAURA_SPIRV_VAL NAMES spirv-val spirv-val.exe PATHS "${ANDROID_NDK}/shader-tools/${shader_host}" NO_DEFAULT_PATH REQUIRED)
find_program(PIXAURA_SPIRV_DIS NAMES spirv-dis spirv-dis.exe PATHS "${ANDROID_NDK}/shader-tools/${shader_host}" NO_DEFAULT_PATH REQUIRED)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/segmentation_spirv.h"
    COMMAND "${PIXAURA_GLSLC}" --target-env=vulkan1.0 -O0 "${CMAKE_CURRENT_SOURCE_DIR}/segmentation.comp" -o "${CMAKE_CURRENT_BINARY_DIR}/segmentation.spv"
    COMMAND "${PIXAURA_SPIRV_VAL}" --target-env vulkan1.0 "${CMAKE_CURRENT_BINARY_DIR}/segmentation.spv"
    COMMAND "${PIXAURA_SPIRV_DIS}" "${CMAKE_CURRENT_BINARY_DIR}/segmentation.spv" -o "${CMAKE_CURRENT_BINARY_DIR}/segmentation.spvasm"
    COMMAND "${CMAKE_COMMAND}" "-DINPUT=${CMAKE_CURRENT_BINARY_DIR}/segmentation.spvasm" -P "${CMAKE_CURRENT_SOURCE_DIR}/../../../scripts/check-inference-spirv.cmake"
    COMMAND "${CMAKE_COMMAND}" "-DINPUT=${CMAKE_CURRENT_BINARY_DIR}/segmentation.spv" "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/segmentation_spirv.h" -DSYMBOL=segmentation_spirv -P "${CMAKE_CURRENT_SOURCE_DIR}/../../../scripts/embed-spirv.cmake"
    DEPENDS segmentation.comp ../../../scripts/embed-spirv.cmake ../../../scripts/check-inference-spirv.cmake VERBATIM)
add_custom_target(pixaura_inference_shader DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/segmentation_spirv.h")
add_dependencies(pixaura_core pixaura_inference_shader)
target_include_directories(pixaura_core PRIVATE "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(pixaura_core PRIVATE vulkan log)
