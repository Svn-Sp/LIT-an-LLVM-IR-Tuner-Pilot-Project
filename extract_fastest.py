import csv
import json
import sys
from typing import Dict, List, Optional, Tuple

CORRECT_RESULT = 3.130682


def load_beam_search_tree(file_path: str) -> Dict:
    """Load the beam search tree from JSON file."""
    try:
        with open(file_path, "r") as f:
            return json.load(f)
    except Exception as e:
        print(f"Error loading JSON file: {e}")
        sys.exit(1)


def find_fastest_correct_node(
    node: Dict, current_path: List[Dict]
) -> List[Dict] | None:
    """
    Recursively find the fastest correct node and its path.
    Returns: path to the fastest correct node
    """
    best_path = None
    if "result" in node and abs(float(node["result"]) - CORRECT_RESULT) < 1e-6:
        best_path = current_path
    for child in node["children"]:
        if new_path := find_fastest_correct_node(child, current_path.copy() + [child]):
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
    # Load the beam search tree
    tree_file = "beam_search_tree.json"
    print(f"Loading beam search tree from {tree_file}...")
    tree = load_beam_search_tree(tree_file)

    # Find the fastest correct node and its path
    print("Searching for the fastest correct node...")
    path_to_node = find_fastest_correct_node(tree, [])

    print(f"Found fastest correct node:")
    print(f"  Path length: {len(path_to_node)} mutations")

    # Print the path for debugging
    print("\nPath to fastest correct node:")
    for i, mutation in enumerate(path_to_node):
        print(f"  Step {i}: {mutation['mutationType']} - {mutation['decisions']}")

    # Save to CSV
    output_file = "fastest_correct_mutations.csv"
    save_mutations_to_csv(path_to_node, output_file)

    print(f"\nSummary:")
    print(f"  Fastest correct node time: {path_to_node[-1]['avg_time']}")
    print(f"  Number of mutations in path: {len(path_to_node)}")
    print(f"  Output file: {output_file}")
    print(path_to_node[-1])


if __name__ == "__main__":
    main()
