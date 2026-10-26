cmake_minimum_required(VERSION 3.20)
project(BanglaKeyboardSwitcher CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
  add_compile_options(/utf-8 /W3)
  # Static CRT: the TIP is loaded into other programs, so it must not need the VC++ redistributable.
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

add_library(bks_core STATIC core/avro_engine.cpp core/bijoy_engine.cpp)

add_executable(engine_tests tests/test_engines.cpp)
target_link_libraries(engine_tests bks_core)
enable_testing()
add_test(NAME engines COMMAND engine_tests)

if(WIN32)
  add_library(BanglaKeyboardTip SHARED tsf/dllmain.cpp tsf/TextService.cpp tsf/BanglaKeyboardTip.def)
  target_compile_definitions(BanglaKeyboardTip PRIVATE UNICODE _UNICODE)
  target_link_libraries(BanglaKeyboardTip PRIVATE bks_core uuid ole32 advapi32 user32)
endif()
