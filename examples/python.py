from pyminiexact import miniexacts_x

x = miniexacts_x()

a = x.primary("a")
b = x.primary("b")

x.add([a, b])
x.add([a])
x.add([b])

while x.solve() == 10:
    print(f"Solution found! Selected options: {x.selected_options()}")

p = x.problem()
print(f"Field `N` of the generated problem instance: {p.N}")
