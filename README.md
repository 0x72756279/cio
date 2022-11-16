# CIO

Perform IO operations on files using the power of C - hence CIO.

## Installation

Clone this repository:

```sh
git clone https://github.com/0x72756279/cio
```

Pull the dependancies:

```sh
git submodule update --init --recursive
```

Build using:

```sh
make
```

### Build for android

To build for android target the env `NDK_PATH` must
point to a valid android-ndk path.
(i.e. /usr/lib/android-ndk)

Than, override the target for make command:

```sh
make target=android
```
