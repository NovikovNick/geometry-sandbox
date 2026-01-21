add_library(font-awesome-7 INTERFACE IconFontCppHeaders/IconsFontAwesome7.h)
add_library(dependencies::icons ALIAS font-awesome-7)
target_include_directories(font-awesome-7 INTERFACE IconFontCppHeaders)

add_library(nlohmann-json INTERFACE json/nlohmann/json.hpp)
add_library(dependencies::json ALIAS nlohmann-json)
target_include_directories(nlohmann-json INTERFACE json)

add_library(animation-easing INTERFACE easing/easing.h)
add_library(dependencies::easing ALIAS animation-easing)
target_include_directories(animation-easing INTERFACE ${CMAKE_SOURCE_DIR}/third_party/easing)
target_compile_options(animation-easing INTERFACE -w)

add_library(entt INTERFACE entt/entt.hpp)
add_library(dependencies::entt ALIAS entt)
target_include_directories(entt INTERFACE ${CMAKE_SOURCE_DIR}/third_party)

add_library(di INTERFACE boost/di.hpp)
add_library(dependencies::di ALIAS di)
target_include_directories(di INTERFACE ${CMAKE_SOURCE_DIR}/third_party)
target_compile_definitions(di INTERFACE BOOST_DI_CFG_CTOR_LIMIT_SIZE=16) # increase number of injections into the constructor. Default is 10
#target_compile_definitions(di INTERFACE BOOST_DI_CFG_DIAGNOSTICS_LEVEL=2) # Default is 1
