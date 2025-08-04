import json
import math
import os
import sys

from pyvis.network import Network

# Check if JSON file exists
if not os.path.exists(sys.argv[1]):
    print(f"Error: {sys.argv[1]} not found!")
    exit(1)

with open(sys.argv[1], "r") as f:
    data = json.load(f)


def generate_name(dict_):
    name = dict_["mutationType"]
    name += f" | {dict_['avg_time']:.4f} ± {dict_['std_dev_time']:.4f}"
    if type(dict_["result"]) == float:
        name += f" | {dict_['result']:.4f}"
    else:
        name += f" | {dict_['result']}"
    return name


def collect_avg_times(node, times_list):
    """Recursively collect all avg_time values from the tree"""
    if "avg_time" in node:
        times_list.append(node["avg_time"])
    for child in node.get("children", []):
        collect_avg_times(child, times_list)


def calculate_node_size(avg_time, min_time, max_time, top_30_percent_threshold):
    """Calculate node size based on speed (avg_time) - faster = larger
    Emphasizes differences among fast nodes (top 30%) while minimizing differences for slow ones"""
    if max_time == min_time:
        return 40  # Default size if all times are the same

    # Check if this node is in the top 30% (faster than threshold)
    is_top_30 = avg_time <= top_30_percent_threshold

    if is_top_30:
        # For top 30%: use fine granularity with cubic emphasis
        # Normalize within the top 30% range (from threshold to min_time)
        top_range = max_time - top_30_percent_threshold
        if top_range == 0:
            top_normalized = 1.0
        else:
            top_normalized = (max_time - avg_time) / top_range

        # Apply cubic transformation for fine granularity
        final_normalized = top_normalized**3

        # Map to size range >40 (emphasized range for fast nodes)
        return 40 + final_normalized
    else:
        # For bottom 70%: use compressed range with minimal differences
        # Normalize within the bottom 70% range (from max_time to threshold)
        bottom_range = top_30_percent_threshold - min_time
        if bottom_range == 0:
            bottom_normalized = 0.0
        else:
            bottom_normalized = (top_30_percent_threshold - avg_time) / bottom_range

        # Map to size range 20-40 (compressed range for slow nodes)
        return 20 + (bottom_normalized * 20)


def interpolate_correctness_color(distance):
    """Interpolate color based on logarithmic distance to correct result"""
    # Use logarithmic scale to emphasize differences between close results
    if distance == 0:
        return "#00FF00"  # Green for perfect match
    if distance < 0.01:
        return "#FFFF00"  # Yellow for close match
    else:
        return "#FF0000"  # Red for invalid values


# Collect all avg_time values to determine the range
all_times = []
if "children" in data and len(data["children"]) > 0:
    for child in data["children"]:
        collect_avg_times(child, all_times)

min_time = min(all_times)
max_time = max(all_times)

sorted_times = sorted(all_times)
threshold_index = int(len(sorted_times) * 0.3)
top_30_percent_threshold = sorted_times[threshold_index]
print(f"Top 30% speed threshold: {top_30_percent_threshold:.4f}")

# Print statistics for debugging
print(f"Time range: {min_time:.6f} to {max_time:.6f}")
print(f"Number of nodes: {len(all_times)}")

# Create network with hierarchical tree layout
net = Network(height="900px", width="100%", bgcolor="#ffffff", font_color="#000000")
net.set_options("""
var options = {
  "nodes": {
    "color": {
      "background": "#97C2FC",
      "border": "#2B7CE9"
    },
    "font": {
      "size": 11,
      "face": "Arial"
    },
    "shape": "box",
    "margin": 10,
    "scaling": {
      "min": 20,
      "max": 60,
      "label": {
        "enabled": true,
        "min": 8,
        "max": 14
      }
    }
  },
  "edges": {
    "color": {
      "color": "#848484"
    },
    "smooth": {
      "type": "cubicBezier",
      "forceDirection": "none"
    },
    "width": 2,
    "font": {
      "size": 10,
      "face": "Arial",
      "color": "#000000",
      "strokeWidth": 2,
      "strokeColor": "#ffffff"
    }
  },
  "layout": {
    "hierarchical": {
      "enabled": true,
      "levelSeparation": 150,
      "nodeSpacing": 200,
      "treeSpacing": 200,
      "blockShifting": true,
      "edgeMinimization": true,
      "parentCentralization": true,
      "direction": "UD",
      "sortMethod": "directed"
    }
  },
  "physics": {
    "enabled": false
  },
  "interaction": {
    "dragNodes": true,
    "dragView": true,
    "zoomView": true,
    "hover": true
  }
}
""")

# Add root node
net.add_node(0, "Root", color="#FF6B6B", level=0)

id_counter = 1


def add_node_and_children(node, net, parent_id, level=1):
    global id_counter

    # Generate node name
    node_name = generate_name(node)

    # Add node with color based on correctness (distance to correct result)
    result_value = node.get("result", "0")
    if type(result_value) != float:
        result_value = float("inf")
    correctness_color = interpolate_correctness_color(result_value)

    # Calculate node size based on speed (avg_time)
    avg_time = node.get("avg_time", 0.0)
    node_size = calculate_node_size(
        avg_time, min_time, max_time, top_30_percent_threshold
    )

    net.add_node(
        id_counter, node_name, color=correctness_color, level=level, size=node_size
    )

    # Use score for edge label, weight, and title
    score = node.get("score", 0.0)
    net.add_edge(
        parent_id,
        id_counter,
        label=f"{score:.2f}",
        weight=score,
    )

    current_id = id_counter
    id_counter += 1

    # Process children
    for child in node.get("children", []):
        add_node_and_children(child, net, current_id, level + 1)


if "children" in data and len(data["children"]) > 0:
    for child in data["children"]:
        add_node_and_children(child, net, 0)
else:
    print("No children found in the tree data")

# Generate and save HTML
html_content = net.generate_html()
with open("beam_search_tree.html", "w") as f:
    f.write(html_content)
print("Tree visualization saved to beam_search_tree.html")
print("Open the HTML file in your web browser to view the hierarchical tree")
print(
    "Node colors represent correctness: Green = correct, Red = incorrect, Yellow = close"
)
print("Node size represents speed: Larger = faster")
print("Scores are displayed directly on the edges")
