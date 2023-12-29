# A simple sudoku en- and decoder. Use it to solve your problems! Made for
# miniexact: https://github.com/miniexact/miniexact
#
# Usage (with $f being one of the generated problems):
#   python3 sudoku.py < problems.txt
#   miniextract -x $f | python3 sudoku.py $f
#
# Encoding inspiration from:
#   https://www.cs.mcgill.ca/~aassaf9/python/sudoku.txt
#   https://gieseanw.wordpress.com/2011/06/16/solving-sudoku-revisited/
#   https://louisabraham.github.io/articles/exact-cover
#   https://www.stolaf.edu/people/hansonr/sudoku/exactcovermatrix.htm
#
# Format inspiration and testing from:
#   https://github.com/PhoenixSmaug/sudoku
#   https://github.com/nstagman/exact_cover_sudoku

import sys
import argparse

# Order: Digit@Cell, Digit@Row, Digit@Column, Digit@Area
side = 9

primaries = side*side + side*side + side*side + side*side

def cell(x: int, y: int):
    return 1 + 0*side*side + (y * side) + x

def digit_in_row(d: int, y: int):
    return 1*side*side + (y*side) + d

def digit_in_column(d: int, x: int):
    return 2*side*side + (x*side) + d

def digit_in_box(d: int, a: int):
    return 3*side*side + (a*side) + d

def encode_line(x: int, y: int, d: int, i: int):
    return f"{cell(x,y)} {digit_in_row(d,y)} {digit_in_column(d,x)} {digit_in_box(d,i//side)} 0\n";

def encode_sudoku_dimacs(line: str, out: str):
    assert(len(line) == side*side)
    with open(out, 'w') as f:
        f.write(f"p xc {primaries} 0\n")
        for i in range(side*side):
            d = int(line[i])
            x = int(i % side)
            y = int(i // side)
            if d == 0:
                for d in range(1,10):
                    f.write(encode_line(x, y, d, i))
            else:
                f.write(encode_line(x, y, d, i))

# encode_sudoku_dimacs('408905020952000480000248150009063804184000067700184002240800905091350200030092071', 'easy-sudoku.xcc')
# encode_sudoku_dimacs('000000008003000400090020060000079000000061200060502070008000500010000020405000003', '1.xcc')

def encode():
    idx = 0
    for line in sys.stdin:
        line = line.strip()
        out = f"{line}.xc"
        encode_sudoku_dimacs(line, out)
        idx = idx + 1

def print_sudoku(line):
    o = list("0" * side)
    for i in range(side*side):
        d = int(line[i])
        x = int(i % side)
        if x == 0 and i > 0:
            print("".join(o))
        o[x] = str(d)
    print("".join(o))

def decode(path_or_line):
    line = path_or_line.split('.')[0]
    print("Unsolved sudoku:")
    print_sudoku(line)
    print()

    solution = sys.stdin.read().strip()
    options = list(map(int, solution.split(' ')))

    solved = list(line)

    option_idx = 1
    for i in range(side*side):
        d = int(line[i])
        x = int(i % side)
        y = int(i // side)
        if d == 0:
            for d in range(1,10):
                if option_idx in options:
                    solved[i] = str(d)
                option_idx += 1
        else:
            option_idx += 1

    print("Solved sudoku:")
    print_sudoku(''.join(solved))

def main():
    parser = argparse.ArgumentParser(
                        prog='sudoku',
                        description='encodes sudoku lines (left to right, then top to bottom number sequences) into .xc format for miniexact')
    parser.add_argument('line', help="if provided, goes to decode mode. May also be a filename with .xc ending.", nargs='?')
    args = parser.parse_args()

    if args.line:
        decode(args.line)
    else:
        encode()

if __name__ == "__main__":
    main()
