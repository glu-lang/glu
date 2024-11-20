#!/bin/bash

set -e

BUILD_DIR="build"
PROFRAW_FILE="default.profraw"
PROFDATA_FILE="default.profdata"
COVERAGE_DIR="coverage_html"
UNIT_TESTS_EXEC="./${BUILD_DIR}/test/unit_tests"

function error_exit {
    echo "$1" 1>&2
    exit 1
}

if [ ! -d "${BUILD_DIR}" ]; then
    error_exit "Build directory '${BUILD_DIR}' does not exist. Please create it and run cmake to configure the project."
fi

echo "Building the project..."
cmake --build ${BUILD_DIR} --target unit_tests || error_exit "Failed to build the project."

if [ ! -f "${UNIT_TESTS_EXEC}" ]; then
    error_exit "Unit tests executable '${UNIT_TESTS_EXEC}' does not exist. Please ensure the build was successful."
fi

echo "Running unit tests with coverage instrumentation..."
LLVM_PROFILE_FILE="${PROFRAW_FILE}" ${UNIT_TESTS_EXEC} || error_exit "Unit tests execution failed."

echo "Merging profile data..."
llvm-profdata merge -sparse ${PROFRAW_FILE} -o ${PROFDATA_FILE} || error_exit "Failed to merge profile data."

echo "Generating coverage report..."
llvm-cov show ${UNIT_TESTS_EXEC} -instr-profile=${PROFDATA_FILE} -format=html -output-dir=${COVERAGE_DIR} -ignore-filename-regex="(build/_deps|test)/" || error_exit "Failed to generate coverage report."

echo "Coverage report generation completed successfully."
