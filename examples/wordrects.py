#!/usr/bin/env python3
#
# Generate a word puzzle. Words may appear:
#   - left to right,
#   - right to left,
#   - top to bottom,
#   - bottom to top
#   - diagonal (all directions)
#
# Takes the alphabet to use from the command line and inserts missing
# letters. Width and height may be modified from the terminal. Input
# words from stdin. Modify generation parameters at will.

import argparse
from sys import stdin
from miniexact import miniexacts_c

def option(s, O, w, x, y, hint, *options):
    for o in options:
        if isinstance(o, tuple):
            s.add(o[0], o[1])
        elif isinstance(o, int):
            s.add(o)
        else:
            raise ValueError
    n = s.add(0)
    O[n] = (w, x, y, hint)

def build_problem(words: set, alphabet: set, width: int, height: int, dlx: str, count: int):
    s = miniexacts_c()

    wp = {}
    p = {}
    C = {}
    CC = {}
    OPTIONS = {}

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
                # Horizontal words:
                if x + l <= width:
                    option(s, OPTIONS, w, x, y, "left->right", *[wp[w], *[(p[(x + i, y)], C[c]) for i,c in enumerate(w)]])
                    option(s, OPTIONS, w, x, y, "right->left", *[wp[w], *[(p[(x + (l - i - 1), y)], C[c]) for i,c in enumerate(w)]])

                # Vertical words:
                if y + l <= height:
                    option(s, OPTIONS, w, x, y, "top->bottom", *[wp[w], *[(p[(x, y + i)], C[c]) for i,c in enumerate(w)]])
                    option(s, OPTIONS, w, x, y, "bottom->top", *[wp[w], *[(p[(x, y + (l - i - 1))], C[c]) for i,c in enumerate(w)]])

                # Diagonal words:
                if x + l <= width and y + l <= height:
                    option(s, OPTIONS, w, x, y, "topleft->lowerright", *[wp[w], *[(p[(x + i, y + i)], C[c]) for i,c in enumerate(w)]])
                    option(s, OPTIONS, w, x, y, "lowerright->topleft", *[wp[w], *[(p[(x + (l - i - 1), y + (l - i - 1))], C[c]) for i,c in enumerate(w)]])
                    option(s, OPTIONS, w, x, y, "topright->lowerleft", *[wp[w], *[(p[(x + (l - i - 1), y + i)], C[c]) for i,c in enumerate(w)]])
                    option(s, OPTIONS, w, x, y, "lowerleft->topright", *[wp[w], *[(p[(x + i, y + (l - i - 1))], C[c]) for i,c in enumerate(w)]])

    if dlx != "":
        s.write_to_dlx(dlx)
        return
                    
    # Solve, and print a solution
    for i in range(count):
        res = s.solve()
        if res == 20:
            print("No solution found!")
            break
        else:
            print("")
            print("===================================================")
            print(f"Solution {i}")
            print("")
            
            for y in range(height):
                sep = ""
                for x in range(width):
                    pos = p[(x, y)]
                    color = s.item_colors()[pos]
                    char = ' '
                    if color != 0:
                        char = CC[color]
                    print(f"{sep}{char}", end='')
                    sep = "\t"
                print("")
            print("")
            print("Word positions:")
            for sel in s.selected_options():
                (w, x, y, hint) = OPTIONS[sel]
                print(f"  Word \"{w}\" starts at {x}x{y} and goes {hint}")

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
    parser.add_argument("--dlx", help="Print problem as DLX to the given path and stop", type=str,
                        default="")
    parser.add_argument("--count", help="Number of solutions to generate", type=int,
                        default=1)

    args = parser.parse_args()

    width = args.width
    height = args.height
    mixed_case = args.mixed_case or False
    dlx = args.dlx
    count = args.count
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

    build_problem(words, alphabet, width, height, dlx, count)

if __name__ == "__main__":
    main()
