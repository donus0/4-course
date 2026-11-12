"""Линейная регрессия с градиентным спуском."""

from __future__ import annotations

from utils._linear_model import (
    Matrix,
    Vector,
    fit_gradient_descent,
    predict_scores,
    validate_training_options,
)


def _identity(z: float) -> float:
    return z


class LinearRegression:
    """Минимизирует половину средней квадратичной ошибки градиентным спуском."""

    def __init__(self, learning_rate: float = 0.01, epochs: int = 1000):
        validate_training_options(learning_rate, epochs)
        self.learning_rate = learning_rate
        self.epochs = epochs
        self.weights: list[float] = []
        self.bias = 0.0

    def fit(self, X: Matrix, y: Vector) -> LinearRegression:
        self.weights, self.bias = fit_gradient_descent(
            X, y, self.learning_rate, self.epochs, _identity
        )
        return self

    def predict(self, X: Matrix) -> list[float]:
        return predict_scores(X, self.weights, self.bias)


def main() -> None:
    X_regression = [[0, 0], [1, 0], [0, 1], [1, 1], [2, 1], [1, 2]]
    y_regression = [1 + 2 * x1 - 3 * x2 for x1, x2 in X_regression]
    regression = LinearRegression(learning_rate=0.1, epochs=500).fit(
        X_regression, y_regression
    )
    print("Линейная регрессия")
    print("Веса:", [round(weight, 3) for weight in regression.weights])
    print("Свободный член:", round(regression.bias, 3))
    predictions = regression.predict([[4, 0], [2, 3]])
    print("Предсказания для [4, 0] и [2, 3]:", [round(value, 3) for value in predictions])


if __name__ == "__main__":
    main()
