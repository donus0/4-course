"""
Задание 3.
Метод Эйлера для задач Коши. Построить графики приближённых решений
на отрезке [0; 10] с шагом h = 0.025.
"""
import os
import numpy as np
import matplotlib.pyplot as plt

OUT_DIR = os.path.join(os.path.dirname(__file__), "output", "task3")
os.makedirs(OUT_DIR, exist_ok=True)

A, B = 0.0, 10.0
H = 0.025


def euler_scalar(f, y0, a, b, h):
    """Explicit Euler method for a scalar ODE y' = f(x, y), y(a) = y0."""
    n = int(round((b - a) / h))
    x = a + h * np.arange(n + 1)
    y = np.zeros(n + 1)
    y[0] = y0
    for k in range(n):
        y[k + 1] = y[k] + h * f(x[k], y[k])
    return x, y


def euler_system(f, y0, a, b, h):
    """Explicit Euler method for a system y' = f(x, y), y(a) = y0 (vector)."""
    n = int(round((b - a) / h))
    x = a + h * np.arange(n + 1)
    y0 = np.asarray(y0, dtype=float)
    y = np.zeros((n + 1, y0.size))
    y[0] = y0
    for k in range(n):
        y[k + 1] = y[k] + h * np.asarray(f(x[k], y[k]))
    return x, y


def plot_scalar(x, y, title, y_label, exact=None, out_path=None):
    fig, ax = plt.subplots(figsize=(9, 5))
    ax.plot(x, y, "-", color="tab:red", linewidth=1.2, label="приближённое решение (метод Эйлера)")
    if exact is not None:
        ax.plot(x, exact(x), "--", color="black", linewidth=1, label="точное решение")
    ax.set_title(title)
    ax.set_xlabel("x")
    ax.set_ylabel(y_label)
    ax.legend(loc="best", fontsize=8)
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    plt.close(fig)
    print(f"saved {out_path}")


def plot_system(x, y, title, labels, exact=None, out_path=None):
    fig, axes = plt.subplots(1, 2, figsize=(13, 5))

    ax = axes[0]
    for i, label in enumerate(labels):
        ax.plot(x, y[:, i], "-", linewidth=1.2, label=f"{label} (Эйлер)")
        if exact is not None:
            ax.plot(x, exact(x)[:, i], "--", linewidth=1, label=f"{label} (точное)")
    ax.set_title(title)
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.legend(loc="best", fontsize=8)
    ax.grid(True, alpha=0.3)

    ax2 = axes[1]
    ax2.plot(y[:, 0], y[:, 1], "-", color="tab:purple", linewidth=1.2, label="фазовая траектория")
    ax2.set_title("Фазовая плоскость")
    ax2.set_xlabel(labels[0])
    ax2.set_ylabel(labels[1])
    ax2.legend(loc="best", fontsize=8)
    ax2.grid(True, alpha=0.3)

    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    plt.close(fig)
    print(f"saved {out_path}")


def main():
    # a) y' = 1/2 * y, y(0) = 1   =>   y(x) = e^(x/2)
    x, y = euler_scalar(lambda x, y: 0.5 * y, 1.0, A, B, H)
    plot_scalar(x, y, "a) y' = (1/2)y, y(0) = 1", "y",
                exact=lambda x: np.exp(0.5 * x),
                out_path=os.path.join(OUT_DIR, "a_y_half_y.png"))

    # b) y' = 2x + 3y, y(0) = -2  =>  y(x) = -(16/9) e^(3x) - (2/3)x - 2/9
    x, y = euler_scalar(lambda x, y: 2 * x + 3 * y, -2.0, A, B, H)
    exact_b = lambda x: -(16.0 / 9.0) * np.exp(3 * x) - (2.0 / 3.0) * x - 2.0 / 9.0
    plot_scalar(x, y, "b) y' = 2x + 3y, y(0) = -2", "y",
                exact=exact_b,
                out_path=os.path.join(OUT_DIR, "b_y_2x_3y.png"))

    # c) x1' = x2, x2' = -x1, x1(0) = 1, x2(0) = 0  =>  x1 = cos(x), x2 = -sin(x)
    f_c = lambda x, v: np.array([v[1], -v[0]])
    x, y = euler_system(f_c, [1.0, 0.0], A, B, H)
    exact_c = lambda x: np.column_stack([np.cos(x), -np.sin(x)])
    plot_system(x, y, "c) x1' = x2, x2' = -x1, x1(0)=1, x2(0)=0",
                labels=["x1", "x2"], exact=exact_c,
                out_path=os.path.join(OUT_DIR, "c_system.png"))

    # d) x1' = x2, x2' = 4x1, x1(0) = 1, x2(0) = 1
    # characteristic roots +-2 => x1 = A e^{2x} + B e^{-2x}, x1(0)=1, x1'(0)=x2(0)=1
    # A + B = 1, 2A - 2B = 1 => A = 3/4, B = 1/4
    f_d = lambda x, v: np.array([v[1], 4 * v[0]])
    x, y = euler_system(f_d, [1.0, 1.0], A, B, H)
    exact_d = lambda x: np.column_stack([
        0.75 * np.exp(2 * x) + 0.25 * np.exp(-2 * x),
        1.5 * np.exp(2 * x) - 0.5 * np.exp(-2 * x),
    ])
    plot_system(x, y, "d) x1' = x2, x2' = 4x1, x1(0)=1, x2(0)=1",
                labels=["x1", "x2"], exact=exact_d,
                out_path=os.path.join(OUT_DIR, "d_system.png"))


if __name__ == "__main__":
    main()
