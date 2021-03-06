#!/bin/sh

scriptDirectory=`dirname "$0"`

pushd "$scriptDirectory/Nuclex.Support.Native"
./build.sh
popd

pushd "$scriptDirectory/Nuclex.Storage.Native"
./build.sh
popd

pushd "$scriptDirectory/Nuclex.Pixels.Native"
./build.sh
popd
