# MiniExact Changelog

## Version 1.4

  - Installing the package over pip also installs the `miniexact` CLI now.

## Version 1.3.1

  - Support the [DLX format](https://cs.stanford.edu/~knuth/programs/dlx2.w)
    from Donald Knuth directly.
  - Add a `write_to_dlx` function to the Python interface for solver objects.
  - Provide a `-P` option that only prints out the parsed problem in DLX.
  - Provide a `-D` option that makes the parser ignore all guessing of input
    format and only read DLX files.

## Version 1.3

  - Make `add` and `selected_options` refer to the same indices in the
    simplified API.
  - Change error reporting mechanism in the simplified API to print to STDERR
    and send the handle into an error state, it no longer just calls `exit`.
  - Add a nicer example to the README.

## Version 1.2.9

  - Change how Python is included in CMake a bit.

## Version 1.2.8

  - Use cibuildwheel to directly build binary packages for many
    platforms automatically.

## Version 1.2.7

  - Use the system Python version in the publish workflow.

## Version 1.2.6

  - Run the publishing workflow on Ubungu LTS instead of latest. This
    should make the package more available.

## Version 1.2.5

  - Delete the cmeel package during publish workflow execution.

## Version 1.2.4

  - Build Wheel in publishing step.
  - Fix Website release.

## Version 1.2.3

  - Fix tests by fixing Catch2 includes.

## Version 1.2.2

  - Add Catch2 amalgamated sources, so `make test` always works.

## Version 1.2.1

  - Small fix for the Python build.

## Version 1.2

  - This is the first version with a release on PyPI! Enjoy
    downloading it using `pip install miniexact`.
  - Previously, the Python package was called `pyminiexact`. To have
    more consistency, it is now just `miniexact`. The shared object is
    still called `pyminiexact` though, to prevent linking issues.
