import os

import matplotlib.pyplot as plt
import numpy as np

# Create diagrams directory if it doesn't exist
os.makedirs("diagrams", exist_ok=True)

# Parameters
rtog = 1.0
rtmax = 1.25
peak = 0.5


# Score function
def score(rt, res, res_correct=True):
    # Part 1 (0.2 * {...})
    if (res == res_correct) or (rt >= peak):
        val1 = (rtmax - rt) / (rtmax - peak)
    else:
        val1 = (rt - (1 / 5) * rtog) / ((3 / 10) * rtog)

    # clamp between 0 and 1
    val1 = np.clip(val1, 0, 1)

    part1 = 0.2 * val1

    # Part 2 ({...})
    part2 = 0.8 if res == res_correct else 0.0

    return part1 + part2


# Generate rt values
rt_values = np.linspace(0, rtmax, 300)

# Compute scores for correct and wrong results
scores_correct = [score(rt, res=True, res_correct=True) for rt in rt_values]
scores_wrong = [score(rt, res=False, res_correct=True) for rt in rt_values]

# Plot
plt.figure(figsize=(8, 5))
plt.plot(
    rt_values, scores_correct, label="res = res_correct", color="green", linewidth=2
)
plt.plot(
    rt_values,
    scores_wrong,
    label="res ≠ res_correct",
    color="red",
    linewidth=2,
)

# Add vertical markers for rt_og, peak, rt_max
for marker, color, label in zip(
    [rtog, peak, rtmax],
    ["blue", "purple", "orange"],
    [r"$rt_{og}$", "peak", r"$rt_{max}$"],
):
    plt.axvline(marker, color=color, linestyle=":", linewidth=2, label=label)

# Decorations
plt.xlabel(r"$rt$", fontsize=12)
plt.ylabel(r"score$(rt, res)$", fontsize=12)
plt.title("Score Function for Program Mutations", fontsize=14)
plt.legend(fontsize=10)
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()

# Save the figure
output_filename = "diagrams/score_function.png"
plt.savefig(output_filename, dpi=300, bbox_inches="tight")
print(f"Plot saved to {output_filename}")

plt.show()
plt.show()
