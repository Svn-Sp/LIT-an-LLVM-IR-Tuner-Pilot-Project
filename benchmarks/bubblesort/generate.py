#!/usr/bin/env python3
"""Generate input cases for the bubblesort benchmark.

Each case is written to input/<id>/ with:
  - input.json   (array of integers to sort)
  - correct.json (sorted reference output)
"""

import argparse
import json
import random
from pathlib import Path

DEFAULT_SIZE = 5000
DEFAULT_MAX_VALUE = 999999


def generate_input(size: int, max_value: int):
    return [random.randint(0, max_value) for _ in range(size)]


def write_case(case_dir: Path, size: int, max_value: int):
    input_data = generate_input(size, max_value)
    correct = sorted(input_data)

    case_dir.mkdir(parents=True, exist_ok=True)
    with open(case_dir / "input.json", "w", encoding="utf-8") as f:
        json.dump(input_data, f)
    with open(case_dir / "correct.json", "w", encoding="utf-8") as f:
        json.dump(correct, f)


def main():
    parser = argparse.ArgumentParser(description="Generate bubblesort benchmark input cases")
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
        "--size",
        type=int,
        default=DEFAULT_SIZE,
        help=f"Number of integers per case (default: {DEFAULT_SIZE})",
    )
    parser.add_argument(
        "--max-value",
        type=int,
        default=DEFAULT_MAX_VALUE,
        help=f"Maximum integer value inclusive (default: {DEFAULT_MAX_VALUE})",
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
        write_case(case_dir, args.size, args.max_value)

    print("Done.")


if __name__ == "__main__":
    main()
