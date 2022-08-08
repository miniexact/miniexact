Solve exact cover problems and expand upon a flexible ground implementation!

## Compiling

Reqirements:

  - C Compiler (e.g. GCC or Clang)
  - FLEX (on Ubuntu: `sudo apt install flex`)
  - BISON (on Ubuntu: `sudo apt install bison`)
  - make
  - cmake

Create a sub-directory, generate a build script and compile the tool. Use
something like this:

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

By default, a `Release` build is created. To develop the project, using the
`Debug` build is recommended. For this, run cmake using `cmake
.. -DCMAKE_BUILD_TYPE=Debug`.

## Usage

The usage is not completely fleshed out yet, as this tool is still under
development. For preliminary solving, write files in this format:

```
< PRIMARY_ITEM_1 PRIMARY_ITEM_2 PRIMARY_ITEM_3 ... >
[ SECONDARY_ITEM_1 SECONDARY_ITEM_2 ... ]
PRIMARY_ITEM_1 PRIMARY_ITEM_3;
PRIMARY_ITEM_2 SECONDARY_ITEM_1:OPTIONAL_COLOR SECONDARY_ITEM_2;
...
```

To run the tool, call `xccsolve` like this:

```
./xccsolve -x ./input-file-name
```

The `-x` selects algorithm X for solving. Others are available too, see
`--help`.

To directly print the selected options, use `-p`.

To enumerate all possible solutions, use `-e`.

The tool also supports solving multiple input files by adding multiple files as
possitional options.

## Performance

This is the second implementation of Knuth's algorithms I did, this one seems
to be around 30% faster than the last one. This mostly goes back to more
inlining potential and a better parser it seems. More potential may still lie
in-between!

Use the algorithm with the littlest possible features! This way, the most
speed-ups are possible. Pre-processing may also be happening in the future.
