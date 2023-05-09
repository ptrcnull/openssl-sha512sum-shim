#!/bin/sh

hyperfine \
    "./build/sha512sum $1" \
    "/usr/bin/sha512sum $1" \
    "/usr/bin/openssl dgst -sha512 -r $1"