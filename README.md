# Netmap Test

a basic sender and receiver application using netmap [1]

## Install netmap

* check out from `https://github.com/luigirizzo/netmap.git`
* in `/LINUX`: `./configure`, `make`, `make install`
* to enable kernel module: `modprobe netmap`

## Compile example:

    mkdir build && cd build
    cmake ..
    make

## References

[1] [https://github.com/luigirizzo/netmap](https://github.com/luigirizzo/netmap)