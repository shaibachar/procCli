cmake_minimum_required(VERSION 3.16)
project(proccli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.13.0
)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "Use bundled fmt for spdlog" FORCE)

FetchContent_MakeAvailable(spdlog nlohmann_json googletest)

add_library(proccli_lib
  src/collectors.cpp
  src/diagnostics.cpp
  src/normalizer.cpp
  src/ollama_client.cpp
  src/report.cpp
  src/utils.cpp
)

target_include_directories(proccli_lib PUBLIC include)

target_link_libraries(proccli_lib PUBLIC spdlog::spdlog nlohmann_json::nlohmann_json)

add_executable(proccli src/main.cpp)

target_link_libraries(proccli PRIVATE proccli_lib)

enable_testing()

add_executable(proccli_tests
  tests/collector_parsing_test.cpp
  tests/normalizer_test.cpp
  tests/report_test.cpp
)

target_link_libraries(proccli_tests PRIVATE proccli_lib gtest_main)

include(GoogleTest)

gtest_discover_tests(proccli_tests)
