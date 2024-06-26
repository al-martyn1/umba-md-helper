cmake_minimum_required(VERSION 3.21.0)
project(umba-md-pp VERSION 0.1.0 LANGUAGES C CXX)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(PRJ_ROOT "${CMAKE_CURRENT_LIST_DIR}")
set(SRC_ROOT "${PRJ_ROOT}/src")
set(LIB_ROOT "${PRJ_ROOT}/_libs")

include(${PRJ_ROOT}/.cmake/umba.cmake)

include_directories(${LIB_ROOT})

add_subdirectory(${LIB_ROOT}/encoding)
add_subdirectory(${LIB_ROOT}/marty_cpp)
# add_subdirectory(${LIB_ROOT}/marty_pugixml)
add_subdirectory(${LIB_ROOT}/marty_tr)
add_subdirectory(${LIB_ROOT}/marty_yaml_toml_json)
add_subdirectory(${LIB_ROOT}/nlohmann)    # https://json.nlohmann.me/integration/cmake/#external
# add_subdirectory(${LIB_ROOT}/pugixml)
add_subdirectory(${LIB_ROOT}/sfmt)
add_subdirectory(${LIB_ROOT}/umba)
add_subdirectory(${LIB_ROOT}/yaml-cpp)




#-O0 
#set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    #-fdiagnostics-color=always
	#set(CMAKE_CXX_FLAGS "-Wa,-mbig-obj -flto -flto-report  -fno-rtti -Wall -Wextra -Werror -pedantic-errors -Wctor-dtor-privacy -Wextra-semi -Wnon-virtual-dtor -Woverloaded-virtual -Wredundant-decls -Wsign-promo")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  #-fignore-unknown-pragmas
  set(CMAKE_CXX_FLAGS "-Woverloaded-virtual -Wno-ctor-dtor-privacy -Wsign-promo")
  #add_compile_options(-fdiagnostics-color=always)
  #add_compile_options(-Wa,-mbig-obj)
  # -fno-rtti
  #add_compile_options(-Wall -Wno-unknown-pragmas -Wduplicated-branches -Wduplicated-cond -Wlogical-op -Wredundant-decls -Wno-unused-function)
  #add_compile_options(-Wno-unknown-pragmas)
  #-Wno-unknown-pragmas
  #-Wnon-virtual-dtor -Wctor-dtor-privacy
  #add_compile_options(-w)
  #add_compile_options(-w -Wa,-mbig-obj)
  #-fdiagnostics-color=always 
  #-Wall   
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
endif()



##set_source_files_properties("${LIBS_ROOT}/encoding/compact_enc_det/compact_enc_det.cc"           PROPERTIES LANGUAGE C )


add_compile_definitions("YAML_CPP_STATIC_DEFINE")
add_compile_definitions("USE_UMBA_LOUT_COUT")

file(GLOB_RECURSE sources "${SRC_ROOT}/*.cpp")
source_group(TREE "${SRC_ROOT}" PREFIX "Sources" FILES "${sources}")

file(GLOB_RECURSE headers "${SRC_ROOT}/*.h")
source_group(TREE "${SRC_ROOT}" PREFIX "Headers" FILES "${headers}")

set(umba_md_pp_view_resources)
if(WIN32)
    set(umba_md_pp_view_resources ${umba_md_pp_view_resources} "${SRC_ROOT}/umba-md-pp/_umba-md-pp-view.rc")
endif()
source_group(TREE "${SRC_ROOT}" PREFIX "Resources" FILES "${umba_md_pp_view_resources}")




add_executable(gen-lang-detection-tr "${SRC_ROOT}/umba-md-pp/gen-lang-detection-tr.cpp" ${headers})
umba_add_target_options(gen-lang-detection-tr "CONSOLE" "UTF8")
target_link_libraries(gen-lang-detection-tr PRIVATE ws2_32 iphlpapi shell32)
target_link_libraries(gen-lang-detection-tr PRIVATE encoding::encoding marty_cpp::marty_cpp sfmt::sfmt umba::umba)
#target_link_libraries(gen-lang-detection-tr PRIVATE pugixml::pugixml marty_pugixml::marty_pugixml)
target_link_libraries(gen-lang-detection-tr PRIVATE nlohmann_json::nlohmann_json yaml-cpp::yaml-cpp)


add_executable(umba-md-pp "${SRC_ROOT}/umba-md-pp/umba-md-pp.cpp" ${headers})
umba_add_target_options(umba-md-pp "CONSOLE" "UTF8")
target_link_libraries(umba-md-pp PRIVATE ws2_32 iphlpapi shell32)
target_link_libraries(umba-md-pp PRIVATE encoding::encoding marty_cpp::marty_cpp sfmt::sfmt umba::umba)
#target_link_libraries(umba-md-pp PRIVATE pugixml::pugixml marty_pugixml::marty_pugixml)
target_link_libraries(umba-md-pp PRIVATE nlohmann_json::nlohmann_json yaml-cpp::yaml-cpp)


add_executable(umba-md-pp-view "${SRC_ROOT}/umba-md-pp/umba-md-pp-view.cpp" ${headers} ${umba_md_pp_view_resources})
umba_add_target_options(umba-md-pp-view "WINDOWS" "UTF8")
target_compile_definitions(umba-md-pp-view PRIVATE "UMBA_MD_PP_VIEW")
target_link_libraries(umba-md-pp-view PRIVATE ws2_32 iphlpapi shell32)
target_link_libraries(umba-md-pp-view PRIVATE encoding::encoding marty_cpp::marty_cpp sfmt::sfmt umba::umba)
#target_link_libraries(umba-md-pp-view PRIVATE pugixml::pugixml marty_pugixml::marty_pugixml)
target_link_libraries(umba-md-pp-view PRIVATE nlohmann_json::nlohmann_json yaml-cpp::yaml-cpp)


add_executable(umba-md-pp-view-console "${SRC_ROOT}/umba-md-pp/umba-md-pp-view.cpp" ${headers} ${umba_md_pp_view_resources})
umba_add_target_options(umba-md-pp-view-console "CONSOLE" "UTF8")
target_compile_definitions(umba-md-pp-view-console PRIVATE "UMBA_MD_PP_VIEW")
target_compile_definitions(umba-md-pp-view-console PRIVATE "UMBA_MD_PP_VIEW_CONSOLE")
target_link_libraries(umba-md-pp-view-console PRIVATE ws2_32 iphlpapi shell32)
target_link_libraries(umba-md-pp-view-console PRIVATE encoding::encoding marty_cpp::marty_cpp sfmt::sfmt umba::umba)
#target_link_libraries(umba-md-pp-view-console PRIVATE pugixml::pugixml marty_pugixml::marty_pugixml)
target_link_libraries(umba-md-pp-view-console PRIVATE nlohmann_json::nlohmann_json yaml-cpp::yaml-cpp)



#https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#target-usage-requirements


#target_compile_options(Framework PUBLIC -Wa,-mbig-obj)
#-flto




#include(CPack)
#set(CPACK_PROJECT_NAME ${PROJECT_NAME})
#set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

