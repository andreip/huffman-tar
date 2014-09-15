# About

It's a compression tool like `tar czf file.zip file1 file2 ...`, but it does compression
by using the huffman coding trees.

It does not work with directories, only with simple files for simplicity.


# How it works

Follow the below steps and see it working

```bash
# compile huffman project
$ make

# compress similar to tar syntax
$ ./hufftar compress out.huff file1 file2 ...

$ ls -l out.huff

# decompress to folder 1/
$ mkdir 1/
$ ./hufftar extract out.huff 1/

# list the .huff compressed files
$ ./hufftar list out.huff
```

# Get convinced it actually compresses

```bash
$ ./hufftar compress out.huff file1 file2 ...
$ tar cf out.tar file1 file2 ...
# Compare their sizes, out.tar only puts the files file1,file2 together without compression
# While out.huff should be significantly lower.
$ ls -l out.huff out.tar
```
