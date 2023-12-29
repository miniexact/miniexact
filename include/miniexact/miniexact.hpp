/*
    miniexact - Toolset to solve exact cover problems and extensions
    Copyright (C) 2021-2023  Maximilian Heisinger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef MINIEXACT_HPP
#define MINIEXACT_HPP

#include <memory>

#include "miniexact.h"

struct miniexact_problem_deleter {
  void operator()(miniexact_problem* p) const { miniexact_problem_free(p, NULL); }
};

using miniexact_problem_ptr = std::unique_ptr<miniexact_problem, miniexact_problem_deleter>;

#endif
