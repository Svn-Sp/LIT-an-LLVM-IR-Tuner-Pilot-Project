#!/usr/bin/env python3
"""
Script to count distinct mutations from beam search results JSON file.
Two mutations are distinct if their mutation types or decision arrays are different.
"""

import json
import sys
from typing import Set, Tuple


def collect_mutations(node: dict) -> Set[Tuple[str, tuple]]:
    mutations = set()

    if "mutationType" in node and "decisions" in node:
        decisions_tuple = tuple(node["decisions"])
        mutations.add((node["mutationType"], decisions_tuple))
    else:
        print("Node has no mutationType or decisions")

    # Recursively process children
    if "children" in node:
        for child in node["children"]:
            mutations.update(collect_mutations(child))

    return mutations


def print_mutation_details(json_file_path: str):
    with open(json_file_path, "r") as f:
        data = json.load(f)

    mutations = collect_mutations(data)

    print(f"\nFound {len(mutations)} distinct mutations:")
    print("-" * 60)

    # Group by mutation type for better readability
    by_type = {}
    for mutation_type, decisions in mutations:
        if mutation_type not in by_type:
            by_type[mutation_type] = []
        by_type[mutation_type].append(decisions)

    for mutation_type in sorted(by_type.keys()):
        print(f"\n{mutation_type}:")
        for decisions in sorted(by_type[mutation_type]):
            print(f"  Decisions: {list(decisions)}")


print_mutation_details(sys.argv[1])
