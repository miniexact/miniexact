# miniexact

A minimalistic and efficient implementation of Donald Knuth's [exact
cover](https://en.wikipedia.org/wiki/Exact_cover) solving algorithms using his
Dancing Links trick . Currently supporting:

  - Algorithm X
  - Algorithm C
  - Algorithm M
  - SAT Backend (Only on Linux and MacOS)

Features:

  - [Web](https://miniexact.github.io/miniexact/) and CLI version
  - Three input formats: [Knuth's
    DLX](https://cs.stanford.edu/~knuth/programs/dlx2.w), One inspired by Donald
    Knuth's text representation, one by DIMACS from SAT solving.
  - C-code is kept as close to Knuth's description as possible using Macros, but
    has a different memory layout.
  - Extensively hackable
  - No dependencies
  - SWIG Bindings support (if available on the system), see the Python example.
  - [Package on PiPI](https://pypi.org/project/miniexact/) with builds for Linux
    (Intel and ARM), MacOS (Intel and ARM), Windows (Intel and ARM), and
    (manually) Pyodide.

Future Goals:

  - Add the new **Dancing Cells** algorithm based on sparse sets as known from
    Christine Solnon and the newest TAOCP Fascicle 7.
  - More comparisons with other implementations.

## Usage

Either use the [web-version](https://miniexact.github.io/miniexact/) in your
browser, the latest universal APE release, the [version on
PyPI](https://pypi.org/project/miniexact/), or compile yourself. The command
line tools expect the algorithm to use (`-x`, `-c`, or `-m`) and the input
file(s). If multiple files are given (e.g. using your shell's wildcard), each
file is solved separately.

A solution is the list of selected options. You can also print the options as
they were listed in the input file with the `-p` (print) switch.

In order to enumerate all possible solutions, use the `-e` (enumerate) switch.

You can change the heuristic used internally to a naive one, but the MRV
heuristic (the default) is a good choice usually.

## Python API (also usable from C and C++)

The package exposes a *simplified API* to Python and other tools trying to
import it over C or C++. Here is an example using algorithm C to solve an exact
cover with colors problem:

``` python
from miniexact import miniexacts_c

# Initiate Solver
s = miniexacts_c()

# First, add primary items. These calls return integers that you may
# use to reference the items later.
a = s.primary("a")
b = s.primary("b")

# After defining primary items, define secondaries.
c = s.secondary("c")

# Now, add some options

# You can add them using the list-based syntax:
maybe_enabled_1 = s.add([a])

# You can also use multiple calls to add(), ending with add(0):
s.add(a)
maybe_enabled_2 = s.add(0)

# This is how you color secondary items:
color1 = s.color("test")
s.add(c, color1)

# You may also use the string-based input to make things easier:
s.add("c", "test")

s.add(b)

# Adds always return the option index they were added in.
forced_option = s.add(0)

# Now, call solve:
res = s.solve()

# The return code is 10 if a solution was found, otherwise it is 20.
assert res == 10

# You can print the solution directly to STDOUT or extract selected
# options.

s.print_solution()

# The returned option indices always match the indices from the add()
# calls.
assert forced_option in s.selected_options()
assert maybe_enabled_1 in s.selected_options() or maybe_enabled_2 in s.selected_options()

# Each additional call to solve() checks for another solution.
res = s.solve()
s.print_solution()
assert res == 10

# Until there are no more solutions:
res = s.solve()
assert res == 20

# The problem can also be serialized into the DLX format:
s.write_to_dlx("out.dlx")
```

The code above prints out the following two solutions:

``` text
c:test2 c:test2 b
a

c:test2 c:test2 b
a
```

With `a` being the first or second option.

In case of an error, the function returns 0 and sends its handle into an
irrecoverable error state. Errors are also printed to STDERR.

## Knuth Exact Cover Format

This format is inspired by Donald Knuth's notation in /The Art of Computer
Programming Volume 4 Fasicle 5/. You first list all primary (possibly with
multiplicity values) and secondary items, then you list all options. This format
is well readable and easy to generate and parse.

### Example

```text
< a b c d e f g >
c e;
a d g;
b c f;
a d f;
b g;
d e g;
```

### Input Grammar

``` ebnf
problem ::= primary_items [ secondary_items ] { option }
primary_items ::= '<' { primary_item } '>'
primary_item ::= ident [ ':' u [ ';' v ] ]
secondary_items ::= '[' { secondary_item } ']'
secondary_item ::= ident
option ::= { ident [ ':' color ] } ';'
```

### Support for DLX Notation

If you use the [DLX-style
notation](https://cs.stanford.edu/~knuth/programs/dlx2.w), namely having a `|`
symbol to separate primary and secondary items and using newlines to separate
options, the parser behaves the same as if using the format above.

It is not supported to have an item called `p` as the first item in this case,
as this would trigger the DIMACS-inspired format below.

## DIMACS-inspired Format

This format is optimized to be generated by tools and is a combination of the
DIMACS format known from SAT solving and the requirements for Exact Cover
problems. You first define the number of primary and secondary items, then you
list the options below. No item names are supported, as only integers are used.
Colors can be given as negative integers after a secondary item was given.

### Example

``` text
p xcc 2 1
2 3 -1 0
1 3 -1 0
```

### Input Grammar

``` ebnf
problem ::= 'p' ( 'xc' | 'xcc' ) <primary count> <secondary count> options
options ::= { option '0' }
option ::= { primary | secondary }
primary ::= <int>
secondary ::= <int> [ '-'<int> ]
```

## Compiling

Reqirements:

  - C Compiler (e.g. GCC or Clang)
  - make
  - cmake
  - Optional: SWIG and Python 2/3

Create a sub-directory, generate a build script and compile the tool. Use
something like this:

```bash
mkdir build
cd build
cmake ..
make
```

By default, a `Release` build is created. To develop the project, using the
`Debug` build is recommended. For this, run cmake using `cmake ..
-DCMAKE_BUILD_TYPE=Debug`.
