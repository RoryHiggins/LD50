#!/usr/bin/env bash
set -ex

LUA_CLIENT=${1:-./ld50/main.lua}
TARGET=${2:-RELEASE}
KEY=${3:-ANY}

RELEASE_NAME=${TARGET}_$(date +'%Y_%m_%d_%H_%M_%S')
RELEASE_DIR=./build/releases/${RELEASE_NAME}
BUILD_DIR=build/${TARGET}_${KEY}

echo mkdir -p "${RELEASE_DIR}"
mkdir -p "${RELEASE_DIR}"
rm -rf "${RELEASE_DIR}/*"

make TARGET=${TARGET} KEY=${KEY} LUA_CLIENT=${LUA_CLIENT}
cp ${BUILD_DIR}/od_client.exe ${RELEASE_DIR}

cp -r engine ${RELEASE_DIR}
cp -r ld50 ${RELEASE_DIR}
# cp -r examples ${RELEASE_DIR}

if [[ "$OSTYPE" == "win32" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "msys" ]]; then
	cp -r client/win64/bin/*.dll ${RELEASE_DIR}
	cp -r client/win64/${TARGET}/bin/*.dll ${RELEASE_DIR} || true
	cp -r ${BUILD_DIR}/*.dll ${RELEASE_DIR} || true
fi
