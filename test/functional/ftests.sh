#!/bin/bash

set -euo pipefail

GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

failures=0

run_test() {
    local label=$1
    local dir=$2
    local flag=$3

    echo -e "${BLUE}Executing test: ${label}${NC}"
    local command="../../build/tools/gluc/gluc ${dir}/assets/*.glu ${flag}"
    local expected="${dir}/expected.result"
    local output="${dir}/result.tmp"

    eval "${command}" > "${output}"

    if diff -u "${expected}" "${output}"; then
        echo -e "${GREEN}[SUCCESS] ${label} passed!${NC}\n"
    else
        echo -e "${RED}[ERROR] ${label} failed!${NC}\n"
        failures=$((failures + 1))
    fi

    rm "${output}"
}

run_test "AST Printer"   "ASTPrinter"   "--print-ast"
run_test "GIL Printer"   "GILPrinter"   "--print-gil"
run_test "Tokens Printer" "TokensPrinter" "--print-tokens"

if [ "${failures}" -gt 0 ]; then
    echo -e "${RED}Some tests failed. Total failures: ${failures}${NC}"
    exit 1
else
    echo -e "${GREEN}All functional tests passed!${NC}"
fi
