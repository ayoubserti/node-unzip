#!/bin/bash -e

SOURCE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd -P)

pushd $SOURCE_DIR
source setup_build_env.sh
node-gyp clean
node-gyp configure  #--arch=x64 --target=0.37.6 --dist-url=https://atom.io/download/atom-shell 
node-gyp build  #--arch=x64 --target=0.37.6 --dist-url=https://atom.io/download/atom-shell 
cp -f build/Release/unzip.node ./
#rm -rf build
popd

echo "Build succeeded"