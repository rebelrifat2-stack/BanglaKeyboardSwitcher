#!/bin/sh
# Builds and runs the engine unit tests on Linux/macOS/WSL/MSYS2 (needs g++ or clang++).
set -e
cd "$(dirname "$0")/.."
${CXX:-g++} -std=c++17 -Wall -Wextra -o /tmp/bks_tests tests/test_engines.cpp core/avro_engine.cpp core/bijoy_engine.cpp
/tmp/bks_tests
