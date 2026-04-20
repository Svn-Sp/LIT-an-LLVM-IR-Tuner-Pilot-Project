import csv
import json
import sys
from typing import Dict, List


MAX_DEPTH = 2


def load_beam_search_tree(file_path: str) -> Dict:
    """Load the beam search tree from JSON file."""
    try:
        with open(file_path, "r") as f:
            return json.load(f)
    except Exception as e:
        print(f"Error loading JSON file: {e}")
        sys.exit(1)


def find_fastest_correct_node_with_max_depth(
    node: Dict, current_path: List[Dict], current_depth: int, max_depth: int
) -> List[Dict] | None:
    """
    Recursively find the fastest correct node and its path, up to max_depth.

    Depth definition:
    - root node is depth 0
    - direct children are depth 1
    - grandchildren are depth 2
    """
    best_path = None
    if "result" in node and float(node["result"]) == 0:
        best_path = current_path

    # Stop exploring deeper once we've reached the allowed depth.
    if current_depth >= max_depth:
        return best_path

    for child in node["children"]:
        child_path = current_path.copy() + [child]
        if new_path := find_fastest_correct_node_with_max_depth(
            child, child_path, current_depth + 1, max_depth
        ):
            if not best_path or new_path[-1]["avg_time"] < best_path[-1]["avg_time"]:
                best_path = new_path
    return best_path


def save_mutations_to_csv(mutations: List[Dict], output_file: str):
    """Save mutations to CSV file."""
    if not mutations:
        print("No mutations found to save.")
        return

    fieldnames = [
        "step",
        "mutationType",
        "decisions",
        "avg_time",
        "std_dev_time",
        "result",
    ]

    with open(output_file, "w", newline="") as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()

        for i, mutation in enumerate(mutations):
            row = {
                "step": i,
                "mutationType": mutation["mutationType"],
                "decisions": str(mutation["decisions"]),
                "avg_time": mutation["avg_time"],
                "std_dev_time": mutation["std_dev_time"],
                "result": mutation["result"],
            }
            writer.writerow(row)

    print(f"Mutations saved to {output_file}")


def main():
    if len(sys.argv) < 2:
        print("Usage: python tools/extract_simple_fastest.py <tree.json>")
        sys.exit(1)

    # Load the beam search tree
    tree_file = sys.argv[1]
    print(f"Loading beam search tree from {tree_file}...")
    tree = load_beam_search_tree(tree_file)

    # Find the fastest correct node and its path up to MAX_DEPTH.
    print(f"Searching for the fastest correct node up to depth {MAX_DEPTH}...")
    path_to_node = find_fastest_correct_node_with_max_depth(tree, [], 0, MAX_DEPTH)
    if not path_to_node:
        print(f"No correct node found up to depth {MAX_DEPTH}.")
        sys.exit(1)

    print("Found fastest correct node:")
    print(f"  Path length: {len(path_to_node)} mutations")

    # Print the path for debugging
    print("\nPath to fastest correct node:")
    for i, mutation in enumerate(path_to_node):
        print(f"  Step {i}: {mutation['mutationType']} - {mutation['decisions']}")

    # Save to CSV
    output_file = "simple_fastest_correct_mutations.csv"
    save_mutations_to_csv(path_to_node, output_file)

    print("\nSummary:")
    print(f"  Fastest correct node time: {path_to_node[-1]['avg_time']}")
    print(f"  Number of mutations in path: {len(path_to_node)}")
    print(f"  Max depth searched: {MAX_DEPTH}")
    print(f"  Output file: {output_file}")
    print(path_to_node[-1])


if __name__ == "__main__":
    main()
