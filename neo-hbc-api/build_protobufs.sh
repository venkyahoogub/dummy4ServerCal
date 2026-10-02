#!/usr/bin/env bash

C_OUTPUT_DIR="./source/c"
PYTHON_OUTPUT_DIR="./source/python"

if [ ! -d "$C_OUTPUT_DIR" ]; then
  mkdir -p "$C_OUTPUT_DIR"
fi

if [ ! -d "$PYTHON_OUTPUT_DIR" ]; then
  mkdir -p "$PYTHON_OUTPUT_DIR"
fi

# use nanopb for c language output
nanopb --nanopb_opt=-I./ --nanopb_out=$C_OUTPUT_DIR hbc.proto

# all other languages are built with Google protoc
protoc -I=./ --python_out=$PYTHON_OUTPUT_DIR hbc.proto
