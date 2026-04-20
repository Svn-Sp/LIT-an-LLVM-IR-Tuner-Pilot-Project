import os
import sys

import matplotlib
import numpy as np
import pandas as pd
from matplotlib.colors import LinearSegmentedColormap
from matplotlib import rcsetup


def configure_matplotlib_backend():
    if matplotlib.get_backend().lower() != "agg":
        return

    interactive_backends = set(rcsetup.interactive_bk)
    preferred_backends = ("qtagg", "tkagg", "gtk3agg", "gtk4agg", "wxagg", "webagg")

    if not (os.environ.get("DISPLAY") or os.environ.get("WAYLAND_DISPLAY")):
        return

    for backend in preferred_backends:
        if backend not in interactive_backends:
            continue
        try:
            matplotlib.use(backend, force=True)
            if backend == "webagg":
                print("No desktop matplotlib backend found; opening plot via WebAgg.")
            return
        except Exception:
            continue


configure_matplotlib_backend()

import matplotlib.pyplot as plt

data = pd.read_csv(sys.argv[1])

data["Average Duration (s)"] = pd.to_numeric(
    data["Average Duration (s)"], errors="coerce"
)
data["Standard Deviation (s)"] = pd.to_numeric(
    data["Standard Deviation (s)"], errors="coerce"
)
data["Run"] = pd.to_numeric(data["Run"], errors="coerce")
data["Result"] = pd.to_numeric(data["Result"], errors="coerce")


plt.figure(figsize=(12, 6))

no_result_mask = data["Success"] == 0
runs_without_results = data[no_result_mask]
plt.errorbar(
    runs_without_results["Run"],
    runs_without_results["Average Duration (s)"],
    yerr=runs_without_results["Standard Deviation (s)"],
    fmt="o",
    capsize=3,
    ecolor="red",
    markerfacecolor="blue",
    markersize=4,
    label="No Result",
)

runs_with_results = data[~no_result_mask]


# Define color mapping based on result values
def get_color(result):
    if result == 0:
        return "green"  # Green for correct results
    elif result < 0.01:
        return "yellow"  # Yellow for close results
    else:
        return "red"  # Red for incorrect results


colors = [get_color(result) for result in runs_with_results["Result"]]

scatter = plt.scatter(
    runs_with_results["Run"],
    runs_with_results["Average Duration (s)"],
    c=colors,
    s=80,  # Size of markers
    zorder=3,  # Draw on top
    edgecolors="black",
)

# Add a colorbar to show the correctness scale
from matplotlib.patches import Patch

legend_elements = [
    Patch(facecolor="green", label="Correct"),
    Patch(facecolor="yellow", label="Close (deviation <0.01)"),
    Patch(facecolor="red", label="Incorrect"),
    Patch(facecolor="blue", label="No Result"),
]
plt.legend(handles=legend_elements, loc="upper right")

# Add error bars separately for runs with results
for i, row in runs_with_results.iterrows():
    plt.errorbar(
        row["Run"],
        row["Average Duration (s)"],
        yerr=row["Standard Deviation (s)"],
        fmt="none",
        capsize=3,
        ecolor="black",
        alpha=0.5,
    )

# Set plot labels and title
plt.xlabel("Run Number")
plt.ylabel("Average Duration (s)")
plt.title("Execution Time with Standard Deviation (Outliers Removed)")
plt.grid(True, linestyle="--", alpha=0.7)

# Adjust layout
plt.tight_layout()

# Show the plot
plt.show()
