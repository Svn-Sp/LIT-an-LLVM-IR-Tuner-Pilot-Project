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


def y_extent_from_runs(df):
    """Lower/upper bound from durations ± std (finite values only)."""
    y = df["Average Duration (s)"]
    sd = df["Standard Deviation (s)"].fillna(0)
    mask = y.notna()
    if not mask.any():
        return None
    y = y[mask]
    sd = sd[mask]
    lo = float((y - sd).min())
    hi = float((y + sd).max())
    return lo, hi


no_result_mask = data["Success"] == 0
runs_without_results = data[no_result_mask]
runs_with_results = data[~no_result_mask]

extent = y_extent_from_runs(runs_with_results)
if extent is None:
    extent = y_extent_from_runs(data)
if extent is None:
    y_lo, y_hi = 0.0, 1.0
else:
    y_lo, y_hi = extent

span = y_hi - y_lo
# Cap padding at 0.5 s for long-running benchmarks; use ~5% of span when tighter.
pad_bottom = min(0.5, max(0.05 * span, 1e-9))
pad_top = min(0.5, max(0.05 * span, 1e-9))
ylim_lo = y_lo - pad_bottom
ylim_hi = y_hi + pad_top


plt.figure(figsize=(12, 6))
plt.ylim(ylim_lo, ylim_hi)


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
    s=10,  # Size of markers
    zorder=3,  # Draw on top
    edgecolors="black",
)

correct = runs_with_results[runs_with_results["Result"] == 0]
plt.plot(
    correct["Run"],
    correct["Average Duration (s)"],
    color="green",
    linewidth=1.2,
    zorder=2,
)

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

# No-result runs: pin markers just above the axis bottom so y-scaling follows timings only.
vertical_span = ylim_hi - ylim_lo
fail_y = ylim_lo + max(0.02 * vertical_span, 1e-12)
fail_runs = runs_without_results["Run"].to_numpy()
plt.scatter(
    fail_runs,
    np.full_like(fail_runs, fail_y, dtype=float),
    s=10,
    zorder=4,
    marker="o",
    facecolors="blue",
    edgecolors="black",
    label="_nolegend",
)

plt.xlabel("Run Number")
plt.ylabel("Average Duration (s)")
plt.title("Execution Time with Standard Deviation (Outliers Removed)")
plt.grid(True, linestyle="--", alpha=0.7)

# Adjust layout
plt.tight_layout()

# Show the plot
plt.show()
