# Copyright (C) 2018-2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

project (pyopenvino)

if(NOT DEFINED OpenVINO_SOURCE_DIR)
    find_package(OpenVINODeveloperPackage REQUIRED)
endif()

# PYTHON_VERSION_MAJOR and PYTHON_VERSION_MINOR are defined inside pybind11
set(pyversion python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR})
message(STATUS "Python version=${pyversion}")

if(OpenVINO_SOURCE_DIR)
    if(WIN32)
        set(PYTHON_BRIDGE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>/python_api/${pyversion}/openvino)
    else()
        set(PYTHON_BRIDGE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/python_api/${pyversion}/openvino)
    endif()

    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PYTHON_BRIDGE_OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PYTHON_BRIDGE_OUTPUT_DIRECTORY})
    set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY ${PYTHON_BRIDGE_OUTPUT_DIRECTORY})
    set(CMAKE_PDB_OUTPUT_DIRECTORY ${PYTHON_BRIDGE_OUTPUT_DIRECTORY})
endif()

# compile options

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # disable warning: This operator was deprecated and will be removed with v0 operation.
    add_compile_options(/wd4996)
    add_compile_options(/wd4244)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wno-deprecated-register -Wno-range-loop-analysis)
elseif(OV_COMPILER_IS_APPLECLANG)
    add_link_options(-stdlib=libc++)
    add_compile_options(-Wno-unused-value -Wno-range-loop-analysis)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # WA for GCC 7.5 "PYBIND11_NOINLINE inline" warning
    add_compile_options(-Wno-error=attributes)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR OV_COMPILER_IS_CLANG)
    # for proper fix need to update pybind to version which does not use PyEval_InitThreads()
    add_compile_options(-Wno-deprecated-declarations -Wno-undef)
endif()

if(ENABLE_TESTS)
    add_subdirectory(test_utils)
endif()

if(TARGET openvino::frontend::onnx)
    add_subdirectory(frontend/onnx)
endif()

if(TARGET openvino::frontend::tensorflow)
    add_subdirectory(frontend/tensorflow)
endif()

if(TARGET openvino::frontend::paddle)
    add_subdirectory(frontend/paddle)
endif()

# create target

file(GLOB_RECURSE SOURCES core/*.cpp graph/*.cpp frontend/*.cpp utils/*cpp pyopenvino.cpp)
list(FILTER SOURCES EXCLUDE REGEX frontend/onnx|tensorflow|paddle/* )

pybind11_add_module(${PROJECT_NAME} MODULE NO_EXTRAS ${SOURCES})

if(TARGET offline_transformations)
    set(OFFLINE_TRANSFORMATIONS_LIB offline_transformations)
else()
    set(OFFLINE_TRANSFORMATIONS_LIB openvino::offline_transformations)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(${PROJECT_NAME} PRIVATE openvino::runtime::dev openvino::runtime ${OFFLINE_TRANSFORMATIONS_LIB})
set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ${ENABLE_LTO} OUTPUT_NAME "_pyopenvino")

addVersionDefines(pyopenvino.cpp CI_BUILD_NUMBER)

# perform copy
if(OpenVINO_SOURCE_DIR)
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../openvino ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            )
endif()

if(OpenVINO_SOURCE_DIR OR OpenVINODeveloperPackage_FOUND)
    ie_python_minimal_api(${PROJECT_NAME})
    add_clang_format_target(${PROJECT_NAME}_clang FOR_TARGETS ${PROJECT_NAME})

    ie_cpack_add_component(${OV_CPACK_COMP_PYTHON_OPENVINO}_${pyversion}
                           HIDDEN)

    install(DIRECTORY ${OpenVINOPython_SOURCE_DIR}/src/openvino
            DESTINATION ${OV_CPACK_PYTHONDIR}
            COMPONENT ${OV_CPACK_COMP_PYTHON_OPENVINO}_${pyversion}
            USE_SOURCE_PERMISSIONS
            PATTERN "test_utils" EXCLUDE)

    install(TARGETS ${PROJECT_NAME}
            DESTINATION ${OV_CPACK_PYTHONDIR}/openvino
            COMPONENT ${OV_CPACK_COMP_PYTHON_OPENVINO}_${pyversion})

    install(DIRECTORY ${OpenVINOPython_SOURCE_DIR}/tests
            DESTINATION tests/${PROJECT_NAME}
            COMPONENT tests EXCLUDE_FROM_ALL)
endif()

if(TARGET ie_wheel)
    add_dependencies(ie_wheel ${PROJECT_NAME})
endif()
