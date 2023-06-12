#!/usr/bin/env bash

test_root=$(readlink -f $(dirname $0))

set -e
SPDK_DIR=$1

sudo $SPDK_DIR/scripts/setup.sh
sudo HUGEMEM=5120 $SPDK_DIR/scripts/setup.sh

export SPDK_HEADER_DIR="$SPDK_DIR/include"
export SPDK_LIB_DIR="$SPDK_DIR/build/lib"
export DPDK_LIB_DIR="$SPDK_DIR/dpdk/build/lib"
export VFIO_LIB_DIR="$SPDK_DIR/libvfio-user/build/release/lib"
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SPDK_LIB_DIR:$DPDK_LIB_DIR:$VFIO_LIB_DIR:"$test_root/passthru"

echo "make"
make -C $test_root hello_world_bdev_shared_combo

echo "run"
#sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SPDK_LIB_DIR:$DPDK_LIB_DIR:$VFIO_LIB_DIR:"$test_root/passthru" $test_root/hello_world/hello_bdev --json $test_root/hello_world/bdev_external.json -b TestPT
sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SPDK_LIB_DIR:$DPDK_LIB_DIR:$VFIO_LIB_DIR:"$test_root/passthru":"$test_root/rout" $test_root/hello_world/hello_bdev --json $test_root/pass.json -m 0x3F

#sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SPDK_LIB_DIR:$DPDK_LIB_DIR:$VFIO_LIB_DIR:"$test_root/passthru":"$test_root/rout" $test_root/hello_world/hello_bdev $test_root/pass.json newthing2 
#-m 0x3
sudo $SPDK_DIR/scripts/setup.sh reset
