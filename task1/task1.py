"""
Задание 1.
Для заданных функций сгенерировать сеточные функции {y_k}, k=1..N,
на заданных отрезках с равномерной сеткой шага h,
и построить графики исходных функций и сеточных функций.
"""
import os
import numpy as np
import matplotlib.pyplot as plt

OUT_DIR = os.path.join(os.path.dirname(__file__), "output", "task1")
os.makedirs(OUT_DIR, exist_ok=True)


def series_exp_even(x, n_terms=60):
    """sum_{n=0}^inf x^(2n) / (2n)!  (== cosh(x)), computed as a truncated series."""
    x = np.asarray(x, dtype=float)
    total = np.zeros_like(x)
    term = np.ones_like(x)  # n = 0 term: x^0 / 0! = 1
    for n in range(n_terms):
        total = total + term
        # move from term_n = x^(2n)/(2n)! to term_{n+1} = x^(2n+2)/(2n+2)!
        term = term * (x ** 2) / ((2 * n + 1) * (2 * n + 2))
    return total


FUNCTIONS = {
    "e^(-x/2)": lambda x: np.exp(-x / 2),
    "sin(3x)": lambda x: np.sin(3 * x),
    "cos^2(5x)": lambda x: np.cos(5 * x) ** 2,
    "sum x^2n/(2n)!": series_exp_even,
}

SEGMENTS = {
    "[0; pi/2]": (0.0, np.pi / 2),
    "[2; 10]": (2.0, 10.0),
    "[-3; 3]": (-3.0, 3.0),
}

STEPS = [0.01, 0.005, 0.001]


def make_grid(a, b, h):
    """Uniform grid on [a, b] with step h (last point may fall slightly short of b)."""
    n = int(round((b - a) / h))
    x = a + h * np.arange(n + 1)
    return x


def main():
    for f_name, f in FUNCTIONS.items():
        for seg_name, (a, b) in SEGMENTS.items():
            x_fine = np.linspace(a, b, 2000)
            y_fine = f(x_fine)

            fig, ax = plt.subplots(figsize=(9, 5))
            ax.plot(x_fine, y_fine, "-", color="black", linewidth=1.2,
                    label=f"{f_name} (исходная функция)")

            markers = ["o", "s", "^"]
            colors = ["tab:red", "tab:blue", "tab:green"]
            for h, marker, color in zip(STEPS, markers, colors):
                x_grid = make_grid(a, b, h)
                y_grid = f(x_grid)
                ax.plot(x_grid, y_grid, marker, markersize=3, linestyle="none",
                         color=color, alpha=0.7, label=f"сеточная функция, h={h}")

            ax.set_title(f"{f_name} на {seg_name}")
            ax.set_xlabel("x")
            ax.set_ylabel("y")
            ax.legend(loc="best", fontsize=8)
            ax.grid(True, alpha=0.3)

            safe_name = f_name.replace("^", "").replace("/", "_").replace(" ", "")
            safe_seg = seg_name.replace(" ", "").replace(";", "_").replace(
                "[", "").replace("]", "").replace("/", "div")
            out_path = os.path.join(OUT_DIR, f"{safe_name}_{safe_seg}.png")
            fig.tight_layout()
            fig.savefig(out_path, dpi=150)
            plt.close(fig)
            print(f"saved {out_path}")


if __name__ == "__main__":
    main()
