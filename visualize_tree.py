import pandas as pd
from pyvis.network import Network
import json
import os


CORRECT_RESULT = 3.130682

# Check if JSON file exists
if not os.path.exists("beam_search_tree.json"):
    print("Error: beam_search_tree.json not found!")
    exit(1)

with open("beam_search_tree.json", "r") as f:
    data = json.load(f)

def generate_name(dict_):
    name = dict_["mutationType"]
    name += f" | {dict_['avg_time']:.4f} ± {dict_['std_dev_time']:.4f}"
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
    normalized = (value - min_val) / (max_val - min_val)
    
    # Interpolate from green (0, 255, 0) to red (255, 0, 0)
    red = int(255 * normalized)
    green = int(255 * (1 - normalized))
    blue = 0
    
    return f"#{red:02X}{green:02X}{blue:02X}"

def calculate_distance_to_correct(result_value):
    """Calculate distance to CORRECT_RESULT"""
    try:
        result_float = float(result_value)
        return abs(result_float - CORRECT_RESULT)
    except (ValueError, TypeError):
        return float('inf')  # Return infinity for invalid values

def interpolate_border_width(distance, min_dist, max_dist):
    """Interpolate border width based on distance to correct result"""
    if max_dist == min_dist:
        return 1  # Default border width if all distances are the same
    
    # Normalize distance between 0 and 1 (closer = smaller distance = thicker border)
    normalized = 1 - ((distance - min_dist) / (max_dist - min_dist))
    
    # Border width from 1 (far) to 8 (close)
    return 1 + (normalized * 7)

# Collect all avg_time values to determine the range
all_times = []
if "children" in data and len(data["children"]) > 0:
    collect_avg_times(data["children"][0], all_times)

if all_times:
    min_time = min(all_times)
    max_time = max(all_times)
    print(f"Time range: {min_time:.4f} to {max_time:.4f}")
else:
    min_time = max_time = 0.0

# Collect all result values to determine distance range
all_results = []
if "children" in data and len(data["children"]) > 0:
    collect_results(data["children"][0], all_results)

if all_results:
    distances = [calculate_distance_to_correct(result) for result in all_results]
    min_dist = min(distances)
    max_dist = max(distances)
    print(f"Distance to correct result range: {min_dist:.6f} to {max_dist:.6f}")
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
    "margin": 10
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
    
    # Add node with color based on avg_time
    avg_time = node.get("avg_time", 0.0)
    color = interpolate_color(avg_time, min_time, max_time)
    
    # Calculate border width based on distance to correct result
    result_value = node.get("result", "0")
    distance = calculate_distance_to_correct(result_value)
    border_width = interpolate_border_width(distance, min_dist, max_dist)
    
    # Create detailed label
    decisions = node.get("decisions", [])
    decisions_str = ",".join(map(str, decisions[:3]))  # Show first 3 decisions
    if len(decisions) > 3:
        decisions_str += "..."
    
    # Add node with color and border width
    net.add_node(id_counter, node_name, color=color, level=level, borderWidth=border_width)
    
    # Use score for edge label
    score = node.get("score", 0.0)
    net.add_edge(parent_id, id_counter, label=f"{score:.2f}")
    
    current_id = id_counter
    id_counter += 1
    
    # Process children
    for child in node.get("children", []):
        add_node_and_children(child, net, current_id, level + 1)

# Process the tree starting from the root
if "children" in data and len(data["children"]) > 0:
    add_node_and_children(data["children"][0], net, 0)
else:
    print("No children found in the tree data")

# Generate and save HTML
html_content = net.generate_html()
with open("beam_search_tree.html", "w") as f:
    f.write(html_content)
print("Tree visualization saved to beam_search_tree.html")
print("Open the HTML file in your web browser to view the hierarchical tree")
print("Node colors represent avg_time: Green = lowest, Red = highest")
print("Node border thickness represents distance to correct result: Thicker = closer to correct")
print("Scores are displayed directly on the edges")

