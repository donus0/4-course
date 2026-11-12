"""Персептрон для бинарной классификации."""

from __future__ import annotations

from collections.abc import Sequence

from utils._linear_model import (
    Matrix,
    linear_score,
    predict_scores,
    training_feature_count,
    validate_binary_labels,
    validate_training_options,
)


class Perceptron:
    """Бинарный классификатор со ступенчатой функцией активации."""

    def __init__(self, learning_rate: float = 0.1, epochs: int = 100):
        validate_training_options(learning_rate, epochs)
        self.learning_rate = learning_rate
        self.epochs = epochs
        self.weights: list[float] = []
        self.bias = 0.0

    def fit(self, X: Matrix, y: Sequence[int]) -> Perceptron:
        feature_count = training_feature_count(X, y)
        validate_binary_labels(y)
        weights = [0.0] * feature_count
        bias = 0.0

        for _ in range(self.epochs):
            mistakes = 0
            for row, target in zip(X, y):
                prediction = int(linear_score(row, weights, bias) >= 0)
                if prediction == target:
                    continue

                correction = self.learning_rate * (target - prediction)
                for j, value in enumerate(row):
                    weights[j] += correction * value
                bias += correction
                mistakes += 1

            if mistakes == 0:
                break

        self.weights, self.bias = weights, bias
        return self

    def predict(self, X: Matrix) -> list[int]:
        return [int(score >= 0) for score in predict_scores(X, self.weights, self.bias)]


def main() -> None:
    X_or = [[0, 0], [0, 1], [1, 0], [1, 1]]
    y_or = [0, 1, 1, 1]

    perceptron = Perceptron().fit(X_or, y_or)
    print("Персептрон на функции ИЛИ")
    print("Классы:", perceptron.predict(X_or))


if __name__ == "__main__":
    main()
