"""Общие вычисления и проверки для линейных моделей."""

from __future__ import annotations

import math
from collections.abc import Callable, Sequence


Vector = Sequence[float]
Matrix = Sequence[Vector]


def validate_training_options(learning_rate: float, epochs: int) -> None:
    if not math.isfinite(learning_rate) or learning_rate <= 0:
        raise ValueError("Скорость обучения должна быть конечной и положительной.")
    if not isinstance(epochs, int) or epochs <= 0:
        raise ValueError("Число эпох должно быть положительным целым числом.")


def training_feature_count(X: Matrix, y: Vector) -> int:
    if len(X) != len(y):
        raise ValueError("Число объектов в X должно совпадать с числом ответов в y.")
    if len(X) == 0 or len(X[0]) == 0:
        raise ValueError("В X нужны хотя бы один объект и один признак.")

    feature_count = len(X[0])
    if any(len(row) != feature_count for row in X):
        raise ValueError("У всех объектов в X должно быть одинаковое число признаков.")
    return feature_count


def validate_binary_labels(y: Vector) -> None:
    if any(label not in (0, 1) for label in y):
        raise ValueError("Метки классов должны быть равны 0 или 1.")


def linear_score(row: Vector, weights: Vector, bias: float) -> float:
    return sum(value * weight for value, weight in zip(row, weights)) + bias


def predict_scores(X: Matrix, weights: Vector, bias: float) -> list[float]:
    if not weights:
        raise RuntimeError("Сначала обучите модель методом fit.")
    if any(len(row) != len(weights) for row in X):
        raise ValueError("Число признаков должно совпадать с обучающей выборкой.")
    return [linear_score(row, weights, bias) for row in X]


def fit_gradient_descent(
    X: Matrix,
    y: Vector,
    learning_rate: float,
    epochs: int,
    activation: Callable[[float], float],
) -> tuple[list[float], float]:
    feature_count = training_feature_count(X, y)
    weights = [0.0] * feature_count
    bias = 0.0
    step = learning_rate / len(X)

    for _ in range(epochs):
        weight_gradient = [0.0] * feature_count
        bias_gradient = 0.0

        for row, target in zip(X, y):
            prediction = activation(linear_score(row, weights, bias))
            # Производная потерь по линейной оценке одинакова для половины MSE
            # и бинарной кросс-энтропии с сигмоидой.
            error = prediction - target
            for j, value in enumerate(row):
                weight_gradient[j] += error * value
            bias_gradient += error

        # Один шаг по среднему градиенту за эпоху.
        for j, gradient in enumerate(weight_gradient):
            weights[j] -= step * gradient
        bias -= step * bias_gradient

    return weights, bias
