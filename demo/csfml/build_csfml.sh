#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

SFML_INCLUDE="${SFML_INCLUDE:-/opt/homebrew/include}"
SFML_LIB="${SFML_LIB:-/opt/homebrew/lib}"

mkdir -p "$BUILD_DIR"

clang++ -std=c++17 -g -O2 \
  -I "$SFML_INCLUDE" \
  -c "$ROOT_DIR/sfml_host.cpp" \
  -o "$BUILD_DIR/sfml_host.o"

export CPATH="$ROOT_DIR:$SFML_INCLUDE:$CPATH"
export GLU_LINKER="${GLU_LINKER:-clang++}"
gluc "$ROOT_DIR/csfml_demo.glu" \
  -Wl,"$BUILD_DIR/sfml_host.o" \
  -Wl,-L"$SFML_LIB" \
  -Wl,-lsfml-graphics \
  -Wl,-lsfml-window \
  -Wl,-lsfml-system \
  -o "$BUILD_DIR/sfml_demo"

printf "Built: %s\n" "$BUILD_DIR/sfml_demo"
