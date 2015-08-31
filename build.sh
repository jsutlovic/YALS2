#!/bin/bash

pushd $(dirname $0) > /dev/null
PROJECT_PATH=$(pwd -P)
popd > /dev/null

rm -rf "$PROJECT_PATH/build" > /dev/null
mkdir "$PROJECT_PATH/build"

pushd "$PROJECT_PATH/build" > /dev/null
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=${1:-Debug} ../
popd > /dev/null
