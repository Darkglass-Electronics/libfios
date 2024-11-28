#!/bin/bash

set -e

target=$(uname -s)
echo $target

case "${target}" in
    "Darwin")
        brew install swig
    ;;
    "Linux")
        pip3 install swig
    ;;
    "MINGW64"*)
    ;;
esac

yarn global add node-gyp
