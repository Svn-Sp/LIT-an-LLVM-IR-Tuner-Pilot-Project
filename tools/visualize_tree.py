import json
import math
import os
import sys
from collections import Counter
from dataclasses import dataclass, field
from typing import Any

import networkx as nx
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
    mutation_label = node.mutation_type or "ROOT"
    score = node.raw.get("score")
    if isinstance(score, (int, float)) and not (
        isinstance(score, float) and math.isnan(score)
    ):
        weight_label = f"{float(score):.4f}"
    else:
        weight_label = "n/a"

    # Keep labels compact and readable for dense trees.
    return f"{mutation_label}\nweight={weight_label}\nruntime={_format_runtime(node.avg_duration)}"


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


def _node_face_edge_color(node: dict[str, Any]) -> tuple[str, str, float]:
    """Return (facecolor, edgecolor, linewidth) for matplotlib from a pyvis node dict."""
    c = node.get("color")
    lw = float(node.get("borderWidth", 1))
    if isinstance(c, dict):
        face = str(c.get("background", "#97C2FC"))
        edge = str(c.get("border", "#2B7CE9"))
        return face, edge, lw
    if isinstance(c, str):
        return c, "#2B7CE9", lw
    return "#97C2FC", "#2B7CE9", lw


def _edge_color_from_vis(edge: dict[str, Any]) -> str:
    c = edge.get("color")
    if isinstance(c, dict):
        return str(c.get("color", "#848484"))
    if isinstance(c, str):
        return c
    return "#848484"


def export_tree_png(net: Network, png_path: str) -> None:
    """Render the pyvis network as a static hierarchical PNG (LR layers)."""
    import numpy as np
    import matplotlib.pyplot as plt
    from matplotlib import patheffects as pe
    from matplotlib.colors import LinearSegmentedColormap
    from matplotlib.patches import Rectangle

    G = nx.DiGraph()
    for n in net.nodes:
        nid = n["id"]
        label = n.get("label", nid)
        if not isinstance(label, str):
            label = str(label)
        level = int(n.get("level", 0))
        face, edge_col, lw = _node_face_edge_color(n)
        G.add_node(nid, label=label, level=level, _face=face, _edge=edge_col, _lw=lw)

    for e in net.edges:
        frm, to = e["from"], e["to"]
        G.add_edge(
            frm,
            to,
            dashes=bool(e.get("dashes")),
            ecolor=_edge_color_from_vis(e),
        )

    if G.number_of_nodes() == 0:
        return

    levels = [int(G.nodes[n]["level"]) for n in G.nodes()]
    layer_counts = Counter(levels)
    max_per_level = max(layer_counts.values(), default=1)
    max_level = max(levels, default=0)
    scale = 3.0 + min(0.8, max_per_level / 18.0)
    pos = nx.multipartite_layout(G, subset_key="level", align="vertical", scale=scale)
    # Narrow overall figure; legend column gets more relative width and larger type.
    w_main = min(28.0, 5.4 + (max_level + 1) * 2.25)
    h = min(36.0, 6.8 + max_per_level * 0.52)
    w_legend = 7.2
    fig, (ax, lax) = plt.subplots(
        1,
        2,
        figsize=(w_main + w_legend, h),
        dpi=150,
        facecolor="white",
        layout="constrained",
        gridspec_kw={"width_ratios": [w_main, w_legend], "wspace": 0.06},
    )
    fig.set_constrained_layout_pads(w_pad=0.03, h_pad=0.04, hspace=0.0, wspace=0.06)

    ax.set_facecolor("white")
    ax.axis("off")
    ax.set_clip_on(False)
    lax.set_facecolor("#fafafa")
    lax.set_xlim(0, 1)
    lax.set_ylim(0, 1)
    lax.axis("off")
    lax.set_clip_on(False)
    for spine in lax.spines.values():
        spine.set_visible(True)
        spine.set_linewidth(0.6)
        spine.set_edgecolor("#bdbdbd")

    nodes = list(G.nodes())
    node_color = [G.nodes[n]["_face"] for n in nodes]
    edgecolors = [G.nodes[n]["_edge"] for n in nodes]
    linewidths = [G.nodes[n]["_lw"] for n in nodes]

    solid = [(u, v) for u, v, d in G.edges(data=True) if not d.get("dashes")]
    dashed = [(u, v) for u, v, d in G.edges(data=True) if d.get("dashes")]

    n_nodes = len(nodes)
    font = max(6, min(10, 15 - n_nodes // 22))

    node_size = max(4500, min(14000, 32000 // max(1, max_per_level)))
    margin = max(22, int(node_size**0.5 * 0.35))
    arr = max(22, int(node_size**0.5 * 0.22))

    if solid:
        nx.draw_networkx_edges(
            G,
            pos,
            edgelist=solid,
            ax=ax,
            arrows=True,
            arrowsize=arr,
            width=1.55,
            edge_color=[G.edges[u, v]["ecolor"] for u, v in solid],
            connectionstyle="arc3,rad=0.04",
            node_size=node_size,
            min_source_margin=margin,
            min_target_margin=margin,
        )
    if dashed:
        nx.draw_networkx_edges(
            G,
            pos,
            edgelist=dashed,
            ax=ax,
            arrows=True,
            arrowsize=max(16, arr - 4),
            width=1.15,
            edge_color=[G.edges[u, v]["ecolor"] for u, v in dashed],
            style="dashed",
            connectionstyle="arc3,rad=0.04",
            node_size=node_size,
            min_source_margin=margin,
            min_target_margin=margin,
        )

    nx.draw_networkx_nodes(
        G,
        pos,
        nodelist=nodes,
        ax=ax,
        node_color=node_color,
        node_shape="o",
        node_size=node_size,
        edgecolors=edgecolors,
        linewidths=linewidths,
    )

    labels = {n: G.nodes[n]["label"] for n in nodes}
    texts = nx.draw_networkx_labels(
        G,
        pos,
        labels,
        ax=ax,
        font_size=font,
        font_family="sans-serif",
        font_color="#1a1a1a",
    )
    for t in texts.values():
        t.set_clip_on(False)
        t.set_path_effects([pe.withStroke(linewidth=2.2, foreground="white", alpha=0.92)])

    xs = np.array([pos[n][0] for n in nodes], dtype=float)
    ys = np.array([pos[n][1] for n in nodes], dtype=float)
    xr = float(xs.max() - xs.min()) or 1.0
    yr = float(ys.max() - ys.min()) or 1.0
    # Room for large circular nodes and multiline labels (not included in pos extent).
    x_pad = max(xr * 0.18, 0.16)
    y_pad = max(yr * 0.40, 0.26 + 0.018 * max_per_level)
    ax.set_xlim(float(xs.min() - x_pad), float(xs.max() + x_pad))
    ax.set_ylim(float(ys.min() - y_pad), float(ys.max() + y_pad))

    for coll in ax.collections:
        coll.set_clip_on(False)
    for coll in ax.patches:
        coll.set_clip_on(False)

    # --- Legend (same semantics as HTML; compact, no inset axes) ---
    rt_cmap = LinearSegmentedColormap.from_list(
        "lit_runtime_legend", [(u, c) for u, c in RUNTIME_PROGRESS_STOPS]
    )
    y_bar_top, h_bar = 0.92, 0.052
    y_bar_bot = y_bar_top - h_bar
    lax.imshow(
        np.linspace(0, 1, 256).reshape(1, -1),
        extent=(0.08, 0.92, y_bar_bot, y_bar_top),
        origin="lower",
        cmap=rt_cmap,
        vmin=0,
        vmax=1,
        aspect="auto",
        zorder=1,
    )
    lax.add_patch(
        Rectangle(
            (0.08, y_bar_bot),
            0.84,
            h_bar,
            fill=False,
            edgecolor="#888888",
            linewidth=0.9,
            zorder=2,
        )
    )
    lax.text(0.08, y_bar_bot - 0.018, "faster", ha="left", va="top", fontsize=14, color="#444444")
    lax.text(0.92, y_bar_bot - 0.018, "slower", ha="right", va="top", fontsize=14, color="#444444")

    lax.text(
        0.5,
        0.988,
        "Legend",
        ha="center",
        va="top",
        fontsize=17,
        fontweight="bold",
        color="#111111",
    )
    if _times:
        runtime_body = (
            "• Green fill: fastest 10% of timed nodes\n"
            "• Black fill: slowest 10%\n"
            "• Between: ramp by relative speed (bar above)"
        )
    else:
        runtime_body = "• No timed nodes in this tree"
    lax.text(
        0.5,
        y_bar_bot - 0.076,
        runtime_body,
        ha="center",
        va="top",
        fontsize=14,
        linespacing=1.55,
        color="#333333",
    )

    lax.text(
        0.5,
        0.49,
        "Correctness (border)",
        ha="center",
        va="top",
        fontsize=15,
        fontweight="semibold",
        color="#222222",
    )
    fill_sample = "#CFD8DC"
    y_corr = 0.39
    # Green = correct (distance 0); red = incorrect (matches node border semantics).
    lax.plot(
        0.26,
        y_corr,
        "o",
        ms=18,
        mfc=fill_sample,
        mec="#00FF00",
        mew=3.6,
        clip_on=False,
    )
    lax.text(
        0.36,
        y_corr,
        "Correct",
        ha="left",
        va="center",
        fontsize=14,
        color="#333333",
    )
    lax.plot(
        0.56,
        y_corr,
        "o",
        ms=18,
        mfc=fill_sample,
        mec="#FF0000",
        mew=3.6,
        clip_on=False,
    )
    lax.text(
        0.66,
        y_corr,
        "Incorrect",
        ha="left",
        va="center",
        fontsize=14,
        color="#333333",
    )

    lax.set_xlim(0, 1)
    lax.set_ylim(0, 1)

    fig.savefig(
        png_path,
        bbox_inches="tight",
        facecolor="white",
        pad_inches=0.45,
    )
    plt.close(fig)


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

# Add stop/add-new option from root as well.
net.add_node(
    id_counter,
    "ADD NEW MUTATION HERE\nweight=0.12",
    color={"background": "#FFFFFF", "border": "#666666"},
    borderWidth=2,
    title="Stop search option weight",
    level=1,
)
net.add_edge(0, id_counter, dashes=True, color="#B0B0B0")
id_counter += 1


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

    # Visualize the stop/add-new-mutation option at this point.
    stop_weight_label = "weight=1" if len(node.children) == 0 else "weight=0.12"
    stop_id = id_counter
    id_counter += 1
    net.add_node(
        stop_id,
        f"ADD NEW MUTATION HERE \n{stop_weight_label}",
        color={"background": "#FFFFFF", "border": "#666666"},
        borderWidth=2,
        title="Stop search option weight",
        level=level + 1,
    )
    net.add_edge(current_id, stop_id, dashes=True, color="#B0B0B0")

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

png_path = "beam_search_tree.png"
try:
    export_tree_png(net, png_path)
    print(f"Tree visualization saved to {png_path}")
except ImportError as e:
    print(f"Skipping PNG export (matplotlib required): {e}")

print("Open the HTML file in your web browser to view the hierarchical tree")
print(
    "Fill: green ≤ fastest 10%, black ≥ slowest 10%, ramp between; "
    "border = correctness"
)
