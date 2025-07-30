import sys

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from matplotlib.colors import LinearSegmentedColormap

FILTERED = True

# Read the CSV file
data = pd.read_csv(sys.argv[1])

# Convert columns to numeric, coercing errors to NaN
data["Average Duration (s)"] = pd.to_numeric(
    data["Average Duration (s)"], errors="coerce"
)
data["Standard Deviation (s)"] = pd.to_numeric(
    data["Standard Deviation (s)"], errors="coerce"
)
data["Run"] = pd.to_numeric(data["Run"], errors="coerce")
data["Result"] = pd.to_numeric(data["Result"], errors="coerce")

# Drop any rows with NaN values in critical columns
data = data.dropna(subset=["Average Duration (s)", "Standard Deviation (s)", "Run"])


# Create the figure and axis
plt.figure(figsize=(12, 6))

# Create a basic plot for runs without results
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

# Create a colorful plot for runs with results
runs_with_results = data[~no_result_mask]
if not runs_with_results.empty:
    # Create binary color mapping: 0 = green (correct), 1 = red (incorrect)
    # Results with value 0 are correct (green), all others are incorrect (red)
    color_values = (runs_with_results["Result"] != 0).astype(int)

    # Custom colormap: green for correct (0), red for incorrect (1)
    cmap = LinearSegmentedColormap.from_list("GreenToRed", ["green", "red"])

    # Plot each point with its color based on correctness
    scatter = plt.scatter(
        runs_with_results["Run"],
        runs_with_results["Average Duration (s)"],
        c=color_values,
        cmap=cmap,
        s=80,  # Size of markers
        zorder=3,  # Draw on top
        edgecolors="black",
        label="With Result",
    )

    # Add a colorbar to show the correctness scale
    cbar = plt.colorbar(scatter)
    cbar.set_label("Result (0 = Correct, 1 = Incorrect)")
    cbar.set_ticks([0, 1])
    cbar.set_ticklabels(["Correct", "Incorrect"])

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
plt.legend()

# Adjust layout
plt.tight_layout()

# Save the figure
plt.savefig("execution_times_plot.png", dpi=300)

# Show the plot
plt.show()
