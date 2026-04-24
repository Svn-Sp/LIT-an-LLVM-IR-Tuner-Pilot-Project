import csv
import json
import sys
from typing import Dict, List, Optional, Tuple


def load_beam_search_tree(file_path: str) -> Dict:
    """Load the beam search tree from JSON file."""
    try:
        with open(file_path, "r") as f:
            return json.load(f)
    except Exception as e:
        print(f"Error loading JSON file: {e}")
        sys.exit(1)


def collect_correct_paths(node: Dict, current_path: List[Dict], results: List[List[Dict]]):
    """Recursively collect paths to all correct nodes, sorted by avg_time ascending."""
    if "result" in node and float(node["result"]) == 0:
        results.append(current_path)
    for child in node["children"]:
        collect_correct_paths(child, current_path.copy() + [child], results)


def find_nth_fastest_correct_node(tree: Dict, n: int) -> List[Dict] | None:
    """
    Find the nth fastest correct node (1-indexed).
    n=1 returns the fastest, n=2 the second fastest, etc.
    """
    results: List[List[Dict]] = []
    collect_correct_paths(tree, [], results)
    if not results:
        return None
    results.sort(key=lambda path: path[-1]["avg_time"])
    if n > len(results):
        print(f"Only {len(results)} correct node(s) found; returning the slowest.")
        return results[-1]
    return results[n - 1]


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
    tree_file = sys.argv[1]
    n = int(sys.argv[2]) if len(sys.argv) > 2 else 1
    print(f"Loading beam search tree from {tree_file}...")
    tree = load_beam_search_tree(tree_file)

    ordinal = {1: "fastest", 2: "2nd fastest", 3: "3rd fastest"}.get(n, f"{n}th fastest")
    print(f"Searching for the {ordinal} correct node...")
    path_to_node = find_nth_fastest_correct_node(tree, n)

    if not path_to_node:
        print("No correct nodes found in the tree.")
        sys.exit(1)

    print(f"Found {ordinal} correct node:")
    print(f"  Path length: {len(path_to_node)} mutations")

    # Print the path for debugging
    print(f"\nPath to {ordinal} correct node:")
    for i, mutation in enumerate(path_to_node):
        print(f"  Step {i}: {mutation['mutationType']} - {mutation['decisions']}")

    # Save to CSV
    output_file = "fastest_correct_mutations.csv"
    save_mutations_to_csv(path_to_node, output_file)

    print(f"\nSummary:")
    print(f"  {ordinal.capitalize()} correct node time: {path_to_node[-1]['avg_time']}")
    print(f"  Number of mutations in path: {len(path_to_node)}")
    print(f"  Output file: {output_file}")
    print(path_to_node[-1])


if __name__ == "__main__":
    main()
