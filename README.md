# README

## Summary

This repository contains code for the server of the calibration tool. Since the client
applicaton will not have a direct connect to the cameras, the primary purpose of the
server is to interact with the cameras, DMD and the database on the behalf of the client.

## Build

Build everything, run tests and static checks...

```sh
./build-all.sh 
```

Build just debug or release

```sh
./build-debug.sh
./build-release
```

Build Container

```sh
./build-container.sh
```

## Checks

Run Tests

```sh
./run-tests.sh
```

Run Static Checks

```sh
./static-checks.sh
```