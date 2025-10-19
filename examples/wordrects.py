#!/usr/bin/env python3
#
# Generate a word puzzle. Words may appear:
#   - left to right,
#   - right to left,
#   - top to bottom,
#   - bottom to top
#
# Takes the alphabet to use from the command line and inserts missing
# letters. Width and height may be modified from the terminal. Input
# words from stdin. Modify generation parameters at will.

import argparse
from sys import stdin
from miniexact import miniexacts_c

def option(s, *options):
    for o in options:
        if isinstance(o, tuple):
            s.add(o[0], o[1])
        elif isinstance(o, int):
            s.add(o)
        else:
            raise ValueError
    s.add(0)

def build_problem(words: set, alphabet: set, width: int, height: int):
    s = miniexacts_c()

    wp = {}
    p = {}
    C = {}
    CC = {}

    # Initiate the problem with all positions and all words.
    for w in words:
        wp[w] = s.primary(w)
    for x in range(width):
        for y in range(height):
            p[(x, y)] = s.secondary(f"c{x}_{y}")
    for c in alphabet:
        C[c] = s.color(c)
        CC[C[c]] = c

    # For every word, step through every possible position
    for w in words:
        l = len(w)
        for x in range(width):
            for y in range(height):
                if width - x >= l:
                    option(s, *[wp[w], *[(p[(x + i, y)], C[c]) for i,c in enumerate(w)]])
                if x >= l:
                    option(s, *[wp[w], *[(p[(x - i, y)], C[c]) for i,c in enumerate(w)]])
                if height - y >= l:
                    option(s, *[wp[w], *[(p[(x, y + i)], C[c]) for i,c in enumerate(w)]])
                if y >= l:
                    option(s, *[wp[w], *[(p[(x, y - i)], C[c]) for i,c in enumerate(w)]])
                    
    # Solve, and print a solution
    res = s.solve()
    if res == 20:
        print("No solution found!")
    else:
        for y in range(height):
            for x in range(width):
                pos = p[(x, y)]
                color = s.item_colors()[pos]
                char = ' '
                if color != 0:
                    char = CC[color]
                print(f"{char}", end='')
            print("")

def main():
    parser = argparse.ArgumentParser(prog="wordrects",
                                   description="A generator for word rectangles")
    parser.add_argument("--width", help="Width of the rectangle", type=int,
                        default=0)
    parser.add_argument("--height", help="Height of the rectangle", type=int,
                        default=0)
    parser.add_argument("-m", "--mixed-case", help="Use mixed-case for words",
                        action='store_true')
    parser.add_argument("-a", "--alphabet", help="The alphabet to use", type=str,
                        default="AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz")

    args = parser.parse_args()

    width = args.width
    height = args.height
    mixed_case = args.mixed_case or False
    words = set()
    alphabet = set()

    for c in args.alphabet:
        if mixed_case and c.islower():
            alphabet.add(c)
        elif c.isupper():
            alphabet.add(c)

    max_word_len = 0

    for line in stdin:
        word = line.strip()
        if not mixed_case:
            word = word.upper()
        if len(word) > max_word_len:
            max_word_len = len(word)
        words.add(word)

        for c in word:
            if c not in alphabet:
                alphabet.add(c)

    if width == 0:
        width = max_word_len
    if height == 0:
        height = max_word_len

    assert width >= max_word_len or height >= max_word_len

    build_problem(words, alphabet, width, height)

if __name__ == "__main__":
    main()
