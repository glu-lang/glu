#!/usr/bin/env bash

SFML_INCLUDE="${SFML_INCLUDE:-/opt/homebrew/include}"
SFML_LIB="${SFML_LIB:-/opt/homebrew/lib}"

export CPATH="$SFML_INCLUDE:$CPATH"
export GLU_LINKER="${GLU_LINKER:-clang++}"
xcrun gluc "./link.glu" \
  -Wl,-L"$SFML_LIB" \
  -Wl,-lsfml-graphics \
  -Wl,-lsfml-window \
  -Wl,-lsfml-system
