#!/bin/bash

set -e

BUILD_DIR="build"
PROFRAW_FILE="default.profraw"
PROFDATA_FILE="default.profdata"
COVERAGE_DIR="coverage_html"
UNIT_TESTS_EXEC="./${BUILD_DIR}/test/unit_tests"

echo "Building the project..."
cmake --build ${BUILD_DIR} --target unit_tests

echo "Running unit tests with coverage instrumentation..."
LLVM_PROFILE_FILE="${PROFRAW_FILE}" ${UNIT_TESTS_EXEC}

echo "Merging profile data..."
llvm-profdata merge -sparse ${PROFRAW_FILE} -o ${PROFDATA_FILE}

echo "Generating coverage report..."
llvm-cov show ${UNIT_TESTS_EXEC} -instr-profile=${PROFDATA_FILE} -format=html -output-dir=${COVERAGE_DIR} -ignore-filename-regex="(build/_deps|test)/"

echo "Coverage report generation completed successfully."
