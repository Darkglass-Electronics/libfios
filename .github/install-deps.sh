#!/bin/bash

set -e

target=$(uname -s)
echo $target

case "${target}" in
    "Darwin")
        brew install python-setuptools swig
    ;;
    "Linux")
        pip3 install swig
    ;;
    "MINGW64"*)
        pip3 install setuptools
    ;;
esac

yarn global add node-gyp@9.4.1
