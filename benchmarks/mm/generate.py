#!/usr/bin/env python3
"""Generate input cases for the mm benchmark.

Each case is written to input/<id>/ with:
  - input1.json  (512x512 matrix)
  - input2.json  (512x512 matrix)
  - correct.json (element-wise product of input1 and input2)
"""

import argparse
import json
import random
from pathlib import Path

N = 128
M = 128


def generate_matrix():
    return [[random.uniform(-1.0, 1.0) for _ in range(M)] for _ in range(N)]


def elementwise_product(matrix1, matrix2):
    return [
        [matrix1[i][j] * matrix2[i][j] for j in range(M)]
        for i in range(N)
    ]


def write_case(case_dir: Path):
    matrix1 = generate_matrix()
    matrix2 = generate_matrix()
    correct = elementwise_product(matrix1, matrix2)

    case_dir.mkdir(parents=True, exist_ok=True)
    files = {
        "input1.json": matrix1,
        "input2.json": matrix2,
        "correct.json": correct,
    }
    for filename, data in files.items():
        with open(case_dir / filename, "w", encoding="utf-8") as f:
            json.dump(data, f)


def main():
    parser = argparse.ArgumentParser(description="Generate mm benchmark input cases")
    parser.add_argument(
        "--count",
        type=int,
        default=1,
        help="Number of cases to generate (default: 1)",
    )
    parser.add_argument(
        "--start",
        type=int,
        default=1,
        help="First case id (default: 1)",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(__file__).resolve().parent / "input",
        help="Directory containing numbered case subfolders",
    )
    args = parser.parse_args()

    for case_id in range(args.start, args.start + args.count):
        case_dir = args.output_dir / str(case_id)
        print(f"Generating case {case_id} -> {case_dir}")
        write_case(case_dir)

    print("Done.")


if __name__ == "__main__":
    main()
