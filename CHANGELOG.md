# MiniExact Changelog

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
