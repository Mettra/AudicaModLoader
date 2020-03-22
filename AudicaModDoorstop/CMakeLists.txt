cmake_minimum_required(VERSION 3.5)

project(AudicaModDoorstop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(AudicaModDoorstop SHARED 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dllmain.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/il2cpp/il2cpp_context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/il2cpp/il2cpp_binding.cpp
)

include(GenerateExportHeader)
generate_export_header(AudicaModDoorstop)

target_include_directories(AudicaModDoorstop PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib")
target_include_directories(AudicaModDoorstop PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../shared")

set_target_properties(AudicaModDoorstop PROPERTIES
    IMPORTED_IMPLIB "${CMAKE_CURRENT_BINARY_DIR}/AudicaModDoorstop.lib"
    IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/AudicaModDoorstop.dll"
)

target_link_libraries(AudicaModDoorstop "${CMAKE_CURRENT_SOURCE_DIR}/lib/funchook.lib" "${CMAKE_CURRENT_SOURCE_DIR}/lib/funchook.lib")
set_target_properties(AudicaModDoorstop PROPERTIES OUTPUT_NAME "IPHLPAPI")
target_compile_definitions(AudicaModDoorstop 
PUBLIC AUDICA_MOD_LOADER
PUBLIC UNICODE
)