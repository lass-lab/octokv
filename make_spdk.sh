#!/usr/bin/env bash

test_root=$(readlink -f $(dirname $0))

set -e
SPDK_DIR=$1

make -C $SPDK_DIR clean
$SPDK_DIR/configure --with-shared --without-isal --without-ocf --disable-asan --disable-unit-tests --disable-tests
make -C $SPDK_DIR -j$(nproc)
