cmake_minimum_required(VERSION 2.8)
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

project(Liss)
include(GetGitRevisionDescription)

set(CMAKE_CXX_FLAGS_RELEASEASSERT "-O3")

include_directories("${PROJECT_SOURCE_DIR}")

#Boost
SET(Boost_USE_STATIC_LIBS    ON)
SET(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.52.0 COMPONENTS filesystem system REQUIRED) 
include_directories(${Boost_INCLUDE_DIRS}) 
link_directories(${Boost_LIBRARY_DIRS})

#LLVM
find_package(LLVM 3.6 COMPONENTS option REQUIRED)
include_directories(${LLVM_INCLUDE_DIRS})
list(REMOVE_ITEM LLVM_CXXFLAGS -fno-exceptions -DNDEBUG -fPIC)
message(STATUS "LLVM_CXXFLAGS=${LLVM_CXXFLAGS}")
message(STATUS "LLVM_LDFLAGS=${LLVM_LDFLAGS}")
add_compile_options(${LLVM_CXXFLAGS} -Wnon-virtual-dtor -Wno-strict-aliasing -Wreturn-type)
set(CLANG_LIBS clangTooling clangFrontend clangDriver clangSerialization clangParse clangSema clangAnalysis clangRewrite clangEdit clangAST clangLex clangBasic)

#z3
include_directories("${PROJECT_SOURCE_DIR}/../libs/z3/src/api" "${PROJECT_SOURCE_DIR}/../libs")
link_directories("${PROJECT_SOURCE_DIR}/../libs/z3/build")

#Limi
include_directories("../libs/Limi")

#for now:
add_definitions(-DSANITY)



#execute_process(COMMAND "${LLVM_BIN_PATH}/llvm-config" --ldflags --libs --system-libs OUTPUT_VARIABLE LLVM_LINKER_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
#string(REGEX REPLACE "\n" " " LLVM_LINKER_FLAGS "${LLVM_LINKER_FLAGS}")
#message(STATUS "LLVM_LINKER_FLAGS=${LLVM_LINKER_FLAGS}")


#git git revision
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GIT.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GIT.cpp" @ONLY)


add_subdirectory(abstraction)
add_subdirectory(clang_interf)
add_subdirectory(actions)
add_subdirectory(synthesis)
add_subdirectory(cfg)
add_subdirectory(placement)

add_executable(liss options.cpp main.cpp "${CMAKE_CURRENT_BINARY_DIR}/GIT.cpp") 

#add_precompiled_header(langinc pch.h
#       FORCEINCLUDE)

target_link_libraries(liss actions synthesis z3 clang_interf placement cfg abstraction ${Boost_LIBRARIES} -fopenmp ${CLANG_LIBS} ${LLVM_LIBRARIES} ${LLVM_LDFLAGS})

#add_executable(test_antichain tests/antichain.cpp)

