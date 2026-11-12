"""Движение груза на пружине с вязким сопротивлением и сухим трением."""

import cmath
from dataclasses import dataclass
from pathlib import Path
from typing import Callable

import matplotlib.pyplot as plt
import numpy as np


MASS = 1.0
STIFFNESS = 4.0
INITIAL_VELOCITY = 1.0
AMPLITUDE = 1.0
FREQUENCY = 2.0
GRAVITY = 9.81
DAMPING_CASES = {"слабое_затухание": 1.0, "сильное_затухание": 5.0}
FRICTION_COEFFICIENTS = (0.0, 0.1)
END_TIME = 10.0
TIME_STEP = 0.005
OUT_DIR = Path(__file__).resolve().parent / "output"

State = tuple[float, float]
Motion = Callable[[float], State]
MotionFactory = Callable[[float, State, int], Motion]


@dataclass(frozen=True)
class Oscillator:
    mass: float
    stiffness: float
    damping: float
    friction: float

    @property
    def friction_limit(self):
        """Модуль силы трения при скольжении и предел трения покоя."""
        return self.friction * self.mass * GRAVITY


@dataclass(frozen=True)
class Force:
    name: str
    label: str
    value: Callable[[float], float]
    particular: Motion


def exponential_particular(model, exponent, amplitude):
    """Возвращает частное решение для вещественной части A exp(λt)."""
    mass, damping, stiffness = model.mass, model.damping, model.stiffness
    denominator = mass * exponent**2 + damping * exponent + stiffness
    power = 0
    if denominator == 0:
        denominator = 2 * mass * exponent + damping
        power = 1
        if denominator == 0:
            denominator = 2 * mass
            power = 2

    def particular(t):
        factor = amplitude * cmath.exp(exponent * t) / denominator
        polynomial = t**power
        derivative = power * t ** (power - 1) if power else 0.0
        return (factor * polynomial).real, (factor * (exponent * polynomial + derivative)).real

    return particular


def make_forces(model):
    """Задаёт четыре внешние силы и соответствующие частные решения."""
    k, h = model.stiffness, model.damping
    return (
        Force("свободное_движение", "0", lambda t: 0.0, lambda t: (0.0, 0.0)),
        Force("линейная_сила", "t-1", lambda t: t - 1,
              lambda t: ((t - 1 - h / k) / k, 1 / k)),
        Force("экспоненциальная_сила", "e^{-t}", lambda t: np.exp(-t),
              exponential_particular(model, -1.0, 1.0)),
        Force("синусоидальная_сила", rf"{AMPLITUDE:g}\sin({FREQUENCY:g}t)",
              lambda t: AMPLITUDE * np.sin(FREQUENCY * t),
              exponential_particular(model, 1j * FREQUENCY, -1j * AMPLITUDE)),
    )


def free_motion(model, x0, v0):
    """Строит решение однородного уравнения с заданными x и v при τ=0."""
    m, h, k = model.mass, model.damping, model.stiffness
    discriminant = h**2 - 4 * m * k
    fast_root = (-h - cmath.sqrt(discriminant)) / (2 * m)
    if discriminant == 0:
        coefficient = v0 - fast_root * x0

        def repeated_root(tau):
            scale = cmath.exp(fast_root * tau)
            x = x0 + coefficient * tau
            return (scale * x).real, (scale * (coefficient + fast_root * x)).real

        return repeated_root

    slow_root = k / (m * fast_root)
    slow_coefficient = (v0 - fast_root * x0) / (slow_root - fast_root)
    fast_coefficient = x0 - slow_coefficient

    def distinct_roots(tau):
        slow = slow_coefficient * cmath.exp(slow_root * tau)
        fast = fast_coefficient * cmath.exp(fast_root * tau)
        return (slow + fast).real, (slow_root * slow + fast_root * fast).real

    return distinct_roots


def analytical_motion(model, force, t0, state, direction):
    """Строит точную формулу движения на участке с постоянным знаком скорости."""
    x0, v0 = state
    offset = -direction * model.friction_limit / model.stiffness
    particular_x, particular_v = force.particular(t0)
    homogeneous = free_motion(model, x0 - particular_x - offset, v0 - particular_v)

    def motion(t):
        x_free, v_free = homogeneous(t - t0)
        x_forced, v_forced = force.particular(t)
        return x_free + x_forced + offset, v_free + v_forced

    return motion


def rk4_motion(model, force, t0, state, direction):
    """Возвращает результат одного шага Рунге–Кутты четвёртого порядка до t."""
    initial = np.asarray(state)
    friction = direction * model.friction_limit

    def derivative(t, state):
        x, v = state
        acceleration = (force.value(t) - model.stiffness * x - model.damping * v - friction) / model.mass
        return np.array([v, acceleration])

    def motion(t):
        dt = t - t0
        half = dt / 2
        k1 = derivative(t0, initial)
        k2 = derivative(t0 + half, initial + half * k1)
        k3 = derivative(t0 + half, initial + half * k2)
        k4 = derivative(t, initial + dt * k3)
        x, v = initial + dt * (k1 + 2 * k2 + 2 * k3 + k4) / 6
        return x, v

    return motion


def sliding_direction(load, limit):
    """Определяет направление старта груза из покоя."""
    if abs(load) <= limit:
        return 0
    return 1 if load > 0 else -1


def crossing_time(start, end, residual):
    """Уточняет бисекцией момент перехода невязки через ноль снизу вверх."""
    while end - start > 1e-12:
        middle = (start + end) / 2
        if residual(middle) > 0:
            end = middle
        else:
            start = middle
    return end


def simulate(times, model, force, initial_velocity, make_motion: MotionFactory):
    """Проходит сетку времени, уточняя остановки и старты внутри каждого шага."""
    t = times[0]
    state = (0.0, initial_velocity)
    states = [state]
    limit = model.friction_limit
    direction = int(np.sign(initial_velocity))

    for end in times[1:]:
        while t < end:
            if limit and not direction:
                spring_force = model.stiffness * state[0]
                direction = sliding_direction(force.value(end) - spring_force, limit)
                if not direction:
                    t = end
                    continue
                t = crossing_time(t, end, lambda s: direction * (force.value(s) - spring_force) - limit)
                if t == end:
                    continue

            motion = make_motion(t, state, direction)
            x, v = motion(end)
            if limit and direction * v < -1e-12:
                t = crossing_time(t, end, lambda s: -direction * motion(s)[1])
                x, _ = motion(t)
                state = (x, 0.0)
                direction = sliding_direction(force.value(t) - model.stiffness * x, limit)
                continue

            if limit and direction * v < 0:
                v = 0.0
            state = (x, v)
            t = end
        states.append(state)
    return np.asarray(states)


def save_plot(times, analytical, numerical, model, force, path):
    """Сохраняет графики смещения и ошибки численного решения."""
    fig = plt.figure(figsize=(11, 6.5), layout="constrained")
    layout = fig.add_gridspec(2, 2, height_ratios=(2, 1))
    exact_ax = fig.add_subplot(layout[0, 0])
    numeric_ax = fig.add_subplot(layout[0, 1], sharex=exact_ax, sharey=exact_ax)
    error_ax = fig.add_subplot(layout[1, :], sharex=exact_ax)
    error = numerical - analytical

    exact_ax.plot(times, analytical, color="black", linewidth=1.3)
    exact_ax.set_title("Аналитическое решение")
    exact_ax.set_ylabel("Смещение x")
    numeric_ax.plot(times, numerical, color="tab:blue", linewidth=1.3)
    numeric_ax.set_title("Численное решение")
    numeric_ax.tick_params(labelleft=False)
    error_ax.plot(times, error, color="tab:red", linewidth=1.1)
    error_ax.set_title("Разность решений")
    error_ax.set_ylabel(r"$x_{\mathrm{числ}}-x_{\mathrm{аналит}}$")
    error_ax.set_xlabel("Время t")
    for ax in (exact_ax, numeric_ax, error_ax):
        ax.grid(True, alpha=0.3)

    fig.suptitle(rf"$f(t)={force.label}$,   $h={model.damping:g}$,   $\mu={model.friction:g}$")
    fig.savefig(path, dpi=150)
    plt.close(fig)
    return np.max(np.abs(error))


def main():
    OUT_DIR.mkdir(exist_ok=True)
    times = np.linspace(0.0, END_TIME, int(np.ceil(END_TIME / TIME_STEP)) + 1)
    for regime, damping in DAMPING_CASES.items():
        for friction in FRICTION_COEFFICIENTS:
            model = Oscillator(MASS, STIFFNESS, damping, friction)
            for force in make_forces(model):
                if model.friction_limit:
                    analytical = simulate(
                        times, model, force, INITIAL_VELOCITY,
                        lambda t, state, direction: analytical_motion(model, force, t, state, direction),
                    )[:, 0]
                else:
                    exact = analytical_motion(model, force, 0.0, (0.0, INITIAL_VELOCITY), 0)
                    analytical = np.array([exact(t)[0] for t in times])
                numerical = simulate(
                    times, model, force, INITIAL_VELOCITY,
                    lambda t, state, direction: rk4_motion(model, force, t, state, direction),
                )[:, 0]
                path = OUT_DIR / f"{regime}_{force.name}_мю_{friction:g}.png"
                max_error = save_plot(times, analytical, numerical, model, force, path)
                print(f"{path.name} — максимальная ошибка {max_error:.3e}")


if __name__ == "__main__":
    main()
