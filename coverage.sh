#!/bin/bash

set -e

PROFRAW_FILE="default.profraw"
PROFDATA_FILE="default.profdata"
COVERAGE_DIR="coverage_html"

BUILD_DIR="build"

UNIT_TESTS_EXEC="./${BUILD_DIR}/test/unit_tests"

FUNCTIONAL_TESTS_EXEC="ftests.sh"
FUNCTIONAL_TESTS_DIR="test/functional"

function error_exit {
    echo "$1" 1>&2
    exit 1
}

if [ ! -d "${BUILD_DIR}" ]; then
    echo "Build directory '${BUILD_DIR}' does not exist. Creating and configuring project with cmake..."
    cmake -Bbuild -DLLVM_ENABLE_ASSERTIONS=1 -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_TARGETS_TO_BUILD="X86" -DENABLE_ASAN=ON -DCMAKE_BUILD_TYPE=Debug -DFROM_SOURCE=0 || error_exit "Failed to configure project."
fi

echo "Building the project..."
cmake --build ${BUILD_DIR} -j$(nproc) || error_exit "Failed to build the project."

if [ ! -f "${UNIT_TESTS_EXEC}" ]; then
    error_exit "Unit tests executable '${UNIT_TESTS_EXEC}' does not exist. Please ensure the build was successful."
fi

echo "Running unit tests with coverage instrumentation..."
LLVM_PROFILE_FILE="${PROFRAW_FILE}" ${UNIT_TESTS_EXEC} || error_exit "Unit tests execution failed."

echo -e "\nRunning functional tests with coverage instrumentation..."
(cd ${FUNCTIONAL_TESTS_DIR} && LLVM_PROFILE_FILE="${PROFRAW_FILE}" ./${FUNCTIONAL_TESTS_EXEC}) || error_exit "Functional tests execution failed."
echo "Merging profile data..."
llvm-profdata merge -sparse ${PROFRAW_FILE} -o ${PROFDATA_FILE} || error_exit "Failed to merge profile data."

echo "Generating coverage report..."
llvm-cov show ${UNIT_TESTS_EXEC} -instr-profile=${PROFDATA_FILE} -format=html -output-dir=${COVERAGE_DIR} -ignore-filename-regex="(build/_deps|test)/" || error_exit "Failed to generate coverage report."

echo "Coverage report generation completed successfully."
