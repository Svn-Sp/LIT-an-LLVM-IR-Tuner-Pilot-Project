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


def collect_results(node, results_list):
    """Recursively collect all result values from the tree"""
    if "result" in node:
        try:
            result_value = float(node["result"])
            results_list.append(result_value)
        except (ValueError, TypeError):
            pass
    for child in node.get("children", []):
        collect_results(child, results_list)


def interpolate_color(value, min_val, max_val):
    """Interpolate color from green (lowest) to red (highest)"""
    if max_val == min_val:
        return "#00FF00"  # Green if all values are the same

    # Normalize value between 0 and 1
    normalized = value / max_val  # Since min_val is always 0

    # Interpolate from green (0, 255, 0) to red (255, 0, 0)
    red = int(255 * normalized)
    green = int(255 * (1 - normalized))
    blue = 0

    return f"#{red:02X}{green:02X}{blue:02X}"


def calculate_node_size(avg_time, min_time, max_time):
    """Calculate node size based on speed (avg_time) - faster = larger"""
    if max_time == min_time:
        return 40  # Default size if all times are the same

    # Normalize time between 0 and 1 (faster = smaller time = larger node)
    normalized = 1 - ((avg_time - min_time) / (max_time - min_time))

    # Node size from 20 (slowest) to 60 (fastest)
    return 20 + (normalized * 40)


def interpolate_correctness_color(distance):
    """Interpolate color based on logarithmic distance to correct result"""
    # Use logarithmic scale to emphasize differences between close results
    if distance == 0:
        return "#00FF00"  # Green for perfect match
    elif distance == float("inf"):
        return "#FF0000"  # Red for invalid values

    # Use logarithmic scale: log(1 + distance) to handle small distances
    log_distance = math.log(1 + distance)

    # Apply a more aggressive logarithmic scaling to make good results very distinct
    # and bad results appear similar
    # Use a smaller max_log_distance to compress the range more
    max_log_distance = 2  # Reduced from 5 to make the effect more pronounced
    normalized = min(1.0, log_distance / max_log_distance)

    # Apply additional compression for very bad results
    # This makes results beyond a certain threshold all appear similar
    if normalized > 0.8:
        normalized = (
            0.8 + (normalized - 0.8) * 0.2
        )  # Compress the top 20% into the top 4%

    # Interpolate from green (correct) to red (incorrect)
    red = int(255 * normalized)
    green = int(255 * (1 - normalized))
    blue = 0

    return f"#{red:02X}{green:02X}{blue:02X}"


# Collect all avg_time values to determine the range
all_times = []
if "children" in data and len(data["children"]) > 0:
    for child in data["children"]:
        collect_avg_times(child, all_times)

if all_times:
    min_time = min(all_times)
    max_time = max(all_times)
    print(f"Time range: {min_time:.4f} to {max_time:.4f}")

    # Calculate threshold for top 30% fastest nodes
    sorted_times = sorted(all_times)
    threshold_index = int(len(sorted_times) * 0.3)
    if threshold_index < len(sorted_times):
        speed_threshold = sorted_times[threshold_index]
        print(f"Top 30% speed threshold: {speed_threshold:.4f}")
    else:
        speed_threshold = max_time
else:
    min_time = max_time = 0.0
    speed_threshold = 0.0

# Collect all result values to determine distance range
all_results = []
if "children" in data and len(data["children"]) > 0:
    for child in data["children"]:
        collect_results(child, all_results)

if all_results:
    min_dist = min(all_results)
    max_dist = max(all_results)
    print(f"Distance to correct result range: {min_dist:.6f} to {max_dist:.6f}")
    print(f"Using logarithmic scaling for deviation visualization")
else:
    min_dist = max_dist = 0.0

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
    node_size = calculate_node_size(avg_time, min_time, max_time)

    # Check if this node is in the top 30% fastest
    is_fast = avg_time <= speed_threshold

    # Create detailed label
    decisions = node.get("decisions", [])
    decisions_str = ",".join(map(str, decisions[:3]))  # Show first 3 decisions
    if len(decisions) > 3:
        decisions_str += "..."

    # Add node with correctness color and speed-based size
    # Add border for top 30% fastest nodes
    if is_fast:
        net.add_node(
            id_counter,
            node_name,
            color=correctness_color,
            level=level,
            size=node_size,
            borderWidth=3,
            borderColor="#0066FF",
        )
    else:
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


# Process the tree starting from the root
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
print("Node colors represent correctness: Green = correct, Red = incorrect")
print("Node size represents speed: Larger = faster")
print("Blue border around nodes indicates top 30% fastest nodes")
print("Scores are displayed directly on the edges")
