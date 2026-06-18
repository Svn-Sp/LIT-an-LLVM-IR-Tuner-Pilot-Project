#!/usr/bin/env python3
"""
Summarize mutation correctness from a beam search tree JSON file.

Correctness means result distance is 0 (matches the reference output).
"""

import json
import sys
from collections import defaultdict
from dataclasses import dataclass
from typing import Iterable


@dataclass(frozen=True)
class Mutation:
    mutation_type: str
    correct: bool


def is_correct(result) -> bool:
    try:
        return float(result) == 0.0
    except (TypeError, ValueError):
        return False


def collect_mutations(node: dict) -> list[Mutation]:
    mutations: list[Mutation] = []

    if "mutationType" in node and "decisions" in node:
        mutations.append(
            Mutation(
                mutation_type=node["mutationType"],
                correct=is_correct(node.get("result")),
            )
        )

    for child in node.get("children", []):
        mutations.extend(collect_mutations(child))

    return mutations


def format_pct(correct: int, total: int) -> str:
    if total == 0:
        return "n/a"
    return f"{100.0 * correct / total:5.1f}%"


def print_summary(title: str, rows: Iterable[tuple[str, int, int]]) -> None:
    print(f"\n{title}")
    print("-" * 72)
    print(f"{'Category':<40} {'Correct':>8} {'Total':>8} {'Rate':>8}")
    for label, correct, total in rows:
        print(f"{label:<40} {correct:>8} {total:>8} {format_pct(correct, total):>8}")


def print_mutation_details(json_file_path: str) -> None:
    with open(json_file_path, "r") as f:
        data = json.load(f)

    mutations = collect_mutations(data)
    if not mutations:
        print("No mutations found in tree.")
        return

    total_correct = sum(1 for m in mutations if m.correct)
    print(f"\nTree nodes with mutations: {len(mutations)}")
    print(
        f"Correct results: {total_correct}/{len(mutations)} "
        f"({format_pct(total_correct, len(mutations))})"
    )

    by_type: dict[str, list[Mutation]] = defaultdict(list)
    for mutation in mutations:
        by_type[mutation.mutation_type].append(mutation)

    type_rows = []
    for mutation_type in sorted(by_type):
        items = by_type[mutation_type]
        correct = sum(1 for m in items if m.correct)
        type_rows.append((mutation_type, correct, len(items)))
    print_summary("By mutation type", type_rows)


def main() -> None:
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <beam_search_tree.json>")
        sys.exit(1)
    print_mutation_details(sys.argv[1])


if __name__ == "__main__":
    main()
