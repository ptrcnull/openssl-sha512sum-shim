#!/bin/sh

hyperfine --warmup 1 --shell=none \
    "./build/sha512sum '$1'" \
    "/usr/bin/sha512sum '$1'" \
    "/usr/bin/openssl dgst -sha512 -r '$1'"