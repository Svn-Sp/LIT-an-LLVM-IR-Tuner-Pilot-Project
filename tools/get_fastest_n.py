import os
import sys

from extract_fastest import load_beam_search_tree, save_mutations_to_csv

FOLDER = "fastest_variants"


def extract_fastest_n_nodes(file, n) -> list[tuple]:
    tree = load_beam_search_tree(file)

    # Traverse the tree and collect all nodes with their avg_time and paths
    def collect_nodes_with_paths(node, current_path, nodes_list):
        # Remove 'children' attribute from node for the path
        node_without_children = dict(node)
        node_without_children.pop("children", None)
        if "avg_time" in node:
            path_with_node = current_path + [node_without_children]
            nodes_list.append((node_without_children, path_with_node))
        for child in node.get("children", []):
            collect_nodes_with_paths(
                child, current_path + [node_without_children], nodes_list
            )

    nodes_with_paths = []
    collect_nodes_with_paths(tree, [], nodes_with_paths)
    nodes_sorted = sorted(nodes_with_paths, key=lambda x: x[0]["avg_time"])
    nodes_filtered = [
        (node, path) for node, path in nodes_sorted if node["result"] == 0
    ]
    return nodes_filtered[:n]


def main(file, n):
    nodes_with_paths = extract_fastest_n_nodes(file, n)
    os.makedirs(FOLDER, exist_ok=True)
    import json

    for i, (node, path) in enumerate(nodes_with_paths):
        save_mutations_to_csv(path[1:], "fastest_correct_mutations.csv")
        os.system("build/rebuild_from_csv")
        os.system(f"mv best.ll {FOLDER}/{i}.ll")
        os.system(f"mv fastest_correct_mutations.csv {FOLDER}/{i}.csv")


if __name__ == "__main__":
    main(sys.argv[1], int(sys.argv[2]))
