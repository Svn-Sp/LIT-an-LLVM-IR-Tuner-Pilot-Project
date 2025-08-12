import json
import random

import numpy as np


def generate_matrix(size):
    """Generate a size x size matrix with random float values"""
    # Generate random float values between -1.0 and 1.0
    matrix = np.random.uniform(-1.0, 1.0, (size, size))

    # Convert to regular Python list for JSON serialization
    matrix_list = matrix.tolist()

    return matrix_list


def save_matrix_to_json(matrix, filename="matrix.json"):
    """Save the matrix to a JSON file"""
    with open(filename, "w") as f:
        json.dump(matrix, f, indent=2)
    print(f"Matrix saved to {filename}")


if __name__ == "__main__":
    # Generate 512x512 matrix
    print("Generating 512x512 matrix...")
    matrix = generate_matrix(512)

    # Save to JSON file
    save_matrix_to_json(matrix, "matrix.json")

    print(f"Matrix shape: {len(matrix)}x{len(matrix[0])}")
    print("Done!")
