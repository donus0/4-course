"""Запускает демонстрации всех трёх алгоритмов."""

from utils._linear_model import Matrix, Vector
from ml_algorithms.linear_regression import LinearRegression, main as demo_linear_regression
from ml_algorithms.perceptron import Perceptron, main as demo_perceptron
from ml_algorithms.sigmoid import LogisticRegression, main as demo_sigmoid, sigmoid


__all__ = [
    "LinearRegression",
    "LogisticRegression",
    "Matrix",
    "Perceptron",
    "Vector",
    "main",
    "sigmoid",
]


def main() -> None:
    demo_linear_regression()
    print()
    demo_perceptron()
    print()
    demo_sigmoid()


if __name__ == "__main__":
    main()
