"""Сигмоида и её применение в логистической регрессии."""

from __future__ import annotations

import math
from collections.abc import Sequence

from utils._linear_model import (
    Matrix,
    fit_gradient_descent,
    predict_scores,
    validate_binary_labels,
    validate_training_options,
)


def sigmoid(z: float) -> float:
    """Вычисляет 1 / (1 + exp(-z)) без переполнения экспоненты."""
    if z >= 0:
        return 1.0 / (1.0 + math.exp(-z))

    exp_z = math.exp(z)
    return exp_z / (1.0 + exp_z)


class LogisticRegression:
    """Обучает бинарный классификатор с сигмоидой и бинарной кросс-энтропией."""

    def __init__(self, learning_rate: float = 0.1, epochs: int = 1000):
        validate_training_options(learning_rate, epochs)
        self.learning_rate = learning_rate
        self.epochs = epochs
        self.weights: list[float] = []
        self.bias = 0.0

    def fit(self, X: Matrix, y: Sequence[int]) -> LogisticRegression:
        validate_binary_labels(y)
        self.weights, self.bias = fit_gradient_descent(
            X, y, self.learning_rate, self.epochs, sigmoid
        )
        return self

    def predict_proba(self, X: Matrix) -> list[float]:
        """Возвращает вероятность класса 1 для каждого объекта."""
        return [sigmoid(score) for score in predict_scores(X, self.weights, self.bias)]

    def predict(self, X: Matrix) -> list[int]:
        return [int(probability >= 0.5) for probability in self.predict_proba(X)]


def main() -> None:
    X_or = [[0, 0], [0, 1], [1, 0], [1, 1]]
    y_or = [0, 1, 1, 1]

    logistic = LogisticRegression().fit(X_or, y_or)
    print("Сигмоида и логистическая регрессия на функции ИЛИ")
    print("Сигмоида для -5, 0 и 5:", [round(sigmoid(z), 3) for z in (-5, 0, 5)])
    probabilities = logistic.predict_proba(X_or)
    print("Вероятности класса 1:", [round(probability, 3) for probability in probabilities])
    print("Классы:", logistic.predict(X_or))


if __name__ == "__main__":
    main()
