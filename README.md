# openssl-sha512sum-shim

> small implementation of sha512sum with openssl evp api to produce the same output as busybox/gnu sha512sum 

it can be ~2x faster than busybox sha512sum! (ymmv)

### usage

```console
$ sha512sum file.txt
89524f30deb37a7c99001321a3e27a7860b481c59e309e9090c27b70536b38f432f2b7f89b2978f16a909d9773cc863344fa385f8aeaf5f8d35fa42aedd066ee  file.txt
$ sha512sum < file.txt
89524f30deb37a7c99001321a3e27a7860b481c59e309e9090c27b70536b38f432f2b7f89b2978f16a909d9773cc863344fa385f8aeaf5f8d35fa42aedd066ee  stdin
```
```console
$ sha512sum file.txt > sums.txt
$ sha512sum -c < sums.txt
file.txt: OK
$ echo content > file.txt
$ sha512sum -c < sums.txt
file.txt: FAILED
sha512sum: WARNING: 1 of 1 computed checksums did NOT match
```
