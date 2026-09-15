"""
Задание 2.
Для заданных функций найти численное значение производной первого порядка
по формуле y' ~= (y_{n+1} - y_n) / h (правая разностная формула),
применённой к сеточным функциям. Построить графики численных производных
для каждого отрезка и параметра h, а также графики производных,
вычисленных аналитически.
"""
import os
import numpy as np
import sympy as sp
import matplotlib.pyplot as plt

OUT_DIR = os.path.join(os.path.dirname(__file__), "output", "task2")
os.makedirs(OUT_DIR, exist_ok=True)

x = sp.symbols("x")

EXPRS = {
    "e^(-x^2/2)": sp.exp(-x ** 2 / 2),
    "sin^3(3x^4/5)": sp.sin(3 * x ** 4 / 5) ** 3,
    "cos^2(x/(x+1))": sp.cos(x / (x + 1)) ** 2,
    "ln(x+sqrt(4+x^2))": sp.log(x + sp.sqrt(4 + x ** 2)),
    "x*arctg(2x)/(x^2+4)": x * sp.atan(2 * x) / (x ** 2 + 4),
}

SEGMENTS = {
    "[0; 1]": (0.0, 1.0),
    "[2; 15]": (2.0, 15.0),
    "[-5; 5]": (-5.0, 5.0),
}

STEPS = [0.01, 0.005]


def make_grid(a, b, h):
    n = int(round((b - a) / h))
    return a + h * np.arange(n + 1)


def main():
    for f_name, expr in EXPRS.items():
        deriv_expr = sp.diff(expr, x)
        f = sp.lambdify(x, expr, "numpy")
        df = sp.lambdify(x, deriv_expr, "numpy")
        print(f"{f_name}: f' = {deriv_expr}")

        for seg_name, (a, b) in SEGMENTS.items():
            # skip segment where x/(x+1) blows up (x = -1) if inside domain
            if f_name == "cos^2(x/(x+1))" and a <= -1 <= b:
                print(f"skip {f_name} on {seg_name}: x=-1 is a singularity")
                continue

            fig, ax = plt.subplots(figsize=(9, 5))

            x_fine = np.linspace(a, b, 2000)
            ax.plot(x_fine, df(x_fine), "-", color="black", linewidth=1.5,
                    label="аналитическая производная")

            colors = ["tab:red", "tab:blue"]
            markers = ["o", "s"]
            for h, color, marker in zip(STEPS, colors, markers):
                x_grid = make_grid(a, b, h)
                y_grid = f(x_grid)
                # right (forward) difference formula: defined for n = 0..N-1,
                # approximates the derivative at x_n
                y_num = (y_grid[1:] - y_grid[:-1]) / h
                x_num = x_grid[:-1]
                ax.plot(x_num, y_num, marker, markersize=2.5, linestyle="none",
                         color=color, alpha=0.7,
                         label=f"численная производная, h={h}")

            ax.set_title(f"f(x) = {f_name}, f'(x) на {seg_name}")
            ax.set_xlabel("x")
            ax.set_ylabel("y'")
            ax.legend(loc="best", fontsize=8)
            ax.grid(True, alpha=0.3)

            safe_name = (f_name.replace("^", "").replace("/", "div")
                         .replace(" ", "").replace("*", ""))
            safe_seg = (seg_name.replace(" ", "").replace(";", "_")
                        .replace("[", "").replace("]", ""))
            out_path = os.path.join(OUT_DIR, f"{safe_name}_{safe_seg}.png")
            fig.tight_layout()
            fig.savefig(out_path, dpi=150)
            plt.close(fig)
            print(f"saved {out_path}")


if __name__ == "__main__":
    main()
