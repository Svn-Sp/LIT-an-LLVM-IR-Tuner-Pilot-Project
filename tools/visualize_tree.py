import json
import math
import os
import sys
from dataclasses import dataclass, field
from typing import Any

from pyvis.network import Network

# Check if JSON file exists
file_name=sys.argv[1]
if not os.path.exists(file_name):
    print(f"Error: {file_name} not found!")
    exit(1)

@dataclass
class TreeNode:
    children: list["TreeNode"] = field(default_factory=list)
    avg_duration: float | None = None
    mutation_type: str | None = None
    result: Any = None
    raw: dict[str, Any] = field(default_factory=dict)

    @staticmethod
    def from_dict(data: dict[str, Any]) -> "TreeNode":
        return TreeNode(
            children=[TreeNode.from_dict(child) for child in data.get("children", [])],
            mutation_type=data.get("mutationType"),
            result=data.get("result"),
            avg_duration=data.get("avg_time", data.get("avgDuration")),
            raw=data,
        )


with open(file_name, "r") as f:
    raw_data = json.load(f)

data = TreeNode.from_dict(raw_data)
def get_all_nodes(root: TreeNode) -> list[TreeNode]:
    """Return a flat list of all nodes in the tree (DFS order)."""
    nodes = []
    def dfs(node):
        nodes.append(node)
        for child in node.children:
            dfs(child)
    dfs(root)
    return nodes

all_nodes = get_all_nodes(data)


def _numeric_runtimes(nodes: list[TreeNode]) -> list[float]:
    out: list[float] = []
    for node in nodes:
        v = node.avg_duration
        if isinstance(v, (int, float)) and not (isinstance(v, float) and math.isnan(v)):
            out.append(float(v))
    return out


_runtime_values = _numeric_runtimes(all_nodes)
_times = sorted(_runtime_values)

if _times:
    _k = max(1, math.ceil(0.1 * len(_times)))
    # Fastest 10%: solid green for t ≤ this (largest time among the fastest decile).
    RUNTIME_GREEN_MAX = _times[_k - 1]
    # Slowest 10%: solid black for t ≥ this (smallest time among the slowest decile).
    RUNTIME_BLACK_MIN = _times[len(_times) - _k]
else:
    RUNTIME_GREEN_MAX = None
    RUNTIME_BLACK_MIN = None

RUNTIME_FAST_COLOR = "#2ECC71"
RUNTIME_SLOW_COLOR = "#000000"

# Ramp u∈[0,1] between green cap and black floor.
RUNTIME_PROGRESS_STOPS: list[tuple[float, str]] = [
    (0.0, "#2ECC71"),
    (0.11, "#7DCE82"),
    (0.22, "#B8E986"),
    (0.33, "#F7DC6F"),
    (0.44, "#F8B739"),
    (0.55, "#E67E22"),
    (0.66, "#E74C3C"),
    (0.77, "#C0392B"),
    (0.88, "#78281F"),
    (0.94, "#3D2C1F"),
    (1.0, "#000000"),
]

def _format_runtime(runtime: float | None) -> str:
    if runtime is None:
        return "n/a"
    return f"{float(runtime):.6g}s"


def generate_name(node: TreeNode):
    name = node.mutation_type or "ROOT"
    if type(node.result) == float:
        name += f" | {str(node.result)[:6]}"
    else:
        name += f" | {str(node.result)[:6]}"
    # Show runtime directly in node label (not only via fill color).
    name += f" | t={_format_runtime(node.avg_duration)}"
    return name


def interpolate_correctness_color(distance):
    """Interpolate color based on logarithmic distance to correct result"""
    # Use logarithmic scale to emphasize differences between close results
    if distance == 0:
        return "#00FF00"  # Green for perfect match
    if distance < 0.001:
        return "#FFFF00"  # Yellow for close match
    else:
        return "#FF0000"  # Red for invalid values


def _hex_to_rgb(hex_color: str) -> tuple[int, int, int]:
    h = hex_color.lstrip("#")
    return (int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16))


def _lerp_hex(color_a: str, color_b: str, t: float) -> str:
    t = max(0.0, min(1.0, t))
    ra, ga, ba = _hex_to_rgb(color_a)
    rb, gb, bb = _hex_to_rgb(color_b)
    return "#{:02X}{:02X}{:02X}".format(
        int(round(ra + (rb - ra) * t)),
        int(round(ga + (gb - ga) * t)),
        int(round(ba + (bb - ba) * t)),
    )


def _color_from_progression_stops(u: float, stops: list[tuple[float, str]]) -> str:
    u = max(0.0, min(1.0, u))
    if u <= stops[0][0]:
        return stops[0][1]
    if u >= stops[-1][0]:
        return stops[-1][1]
    for i in range(len(stops) - 1):
        u0, c0 = stops[i]
        u1, c1 = stops[i + 1]
        if u <= u1:
            span = u1 - u0
            if span <= 0:
                return c1
            t = (u - u0) / span
            return _lerp_hex(c0, c1, t)
    return stops[-1][1]


def _css_linear_gradient_from_stops(stops: list[tuple[float, str]], direction: str = "to right") -> str:
    parts = [f"{color} {frac * 100:.1f}%" for frac, color in stops]
    return f"linear-gradient({direction}, {', '.join(parts)})"


def runtime_gradient_color(runtime: float | None) -> str:
    if runtime is None:
        return "#B0BEC5"
    t = float(runtime)
    if not _times:
        return RUNTIME_FAST_COLOR
    # Green before black so tied runtimes at both cutoffs count as green.
    if t <= RUNTIME_GREEN_MAX:
        return RUNTIME_FAST_COLOR
    if t >= RUNTIME_BLACK_MIN:
        return RUNTIME_SLOW_COLOR
    hi = RUNTIME_BLACK_MIN
    lo = RUNTIME_GREEN_MAX
    if hi <= lo:
        return _color_from_progression_stops(1.0, RUNTIME_PROGRESS_STOPS)
    u = (t - lo) / (hi - lo)
    return _color_from_progression_stops(max(0.0, min(1.0, u)), RUNTIME_PROGRESS_STOPS)


def _inject_runtime_legend(html: str) -> str:
    grad = _css_linear_gradient_from_stops(RUNTIME_PROGRESS_STOPS)
    if _times:
        g, b = f"{RUNTIME_GREEN_MAX:.6g}", f"{RUNTIME_BLACK_MIN:.6g}"
        desc = (
            f"Green if ≤ {g}s (fastest 10% of timed nodes). "
            f"Black if ≥ {b}s (slowest 10%). Ramp in between. "
            f"Border = correctness."
        )
    else:
        desc = "No timed nodes in this tree."
    legend = f"""
<div id="runtime-legend" style="position:fixed;top:12px;right:12px;z-index:9999;
  background:rgba(255,255,255,0.95);border:1px solid #ccc;border-radius:6px;
  padding:10px 12px;max-width:300px;font-family:Arial,sans-serif;font-size:12px;
  box-shadow:0 2px 8px rgba(0,0,0,0.12);">
  <div style="font-weight:bold;margin-bottom:6px;">Runtime (fill)</div>
  <div style="height:14px;border-radius:4px;border:1px solid #999;background:{grad};"></div>
  <div style="margin-top:8px;line-height:1.4;color:#333;">{desc}</div>
</div>
"""
    if "</body>" in html:
        return html.replace("</body>", legend + "\n</body>", 1)
    return html + legend


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
      "direction": "LR",
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


def add_node_and_children(node: TreeNode, net, parent_id, level=1):
    global id_counter

    # Generate node name
    node_name = generate_name(node)

    # Add node with color based on correctness (distance to correct result)
    result_value = node.result if node.result is not None else "0"
    if type(result_value) != float:
        result_value = float("inf")
    correctness_color = interpolate_correctness_color(result_value)
    runtime_color = runtime_gradient_color(node.avg_duration)

    net.add_node(
        id_counter,
        node_name,
        color={"background": runtime_color, "border": correctness_color},
        borderWidth=3,
        title=f"runtime={_format_runtime(node.avg_duration)}",
        level=level,
    )
    net.add_edge(parent_id, id_counter)

    current_id = id_counter
    id_counter += 1

    # Process children
    for child in node.children:
        add_node_and_children(child, net, current_id, level + 1)


if data and data.children:
    for child in data.children:
        add_node_and_children(child, net, 0)
else:
    print("No children found in the filtered tree data")

# Generate and save HTML
html_content = _inject_runtime_legend(net.generate_html())
with open("beam_search_tree.html", "w") as f:
    f.write(html_content)
print("Tree visualization saved to beam_search_tree.html")
print("Open the HTML file in your web browser to view the hierarchical tree")
print(
    "Fill: green ≤ fastest 10%, black ≥ slowest 10%, ramp between; "
    "border = correctness"
)
