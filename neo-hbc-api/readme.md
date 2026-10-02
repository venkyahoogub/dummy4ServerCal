# Neo HBC API

This repository contains the Protobuf interface source code for data interchange with the Head Board Controller embedded server.

Pre-built source code libraries are available in the source directory for all languages. 

### Installing dependencies

The Google Protobuf compiler (protoc) must be installed in order to build the proto files.

*MacOS:*

```
brew install protobuf
```

*Linux:*
```
apt install protobuf-compiler
```

In addition to the Google Protobuf compiler, the NanoPb compiler must also be installed.
NanoPb can be obtained from Bitbucket.org by cloning the following repo.

```
git clone git@bitbucket.org:glaukos-east/nanopb.git
```

After cloning, create a symlinks to the compiler. Replace <nanopb_dir> with the directory path to nanopb.

```
ln -s <nanopb_dir>/generator/protoc-gen-nanopb /usr/local/bin/protoc-gen-nanopb
ln -s <nanopb_dir>/generator/protoc /usr/local/bin/nanopb
```

### Building proto files

```
chmod +x build_protobufs.sh
./build_protobufs.sh
```
