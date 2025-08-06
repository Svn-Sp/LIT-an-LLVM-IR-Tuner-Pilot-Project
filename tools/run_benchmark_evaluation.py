import csv
import json
import os
import statistics
import subprocess
import sys
import time
from dataclasses import dataclass

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

RUNS = 1
BASELINE_RUNS = 10


def get_result_folder_name(benchmark_name):
    return f"eval_results/{benchmark_name}"


@dataclass
class RuntimeMeasurementResult:
    time: float
    min_time: float
    max_time: float

    def to_dict(self):
        return {
            "time": self.time,
            "min_time": self.min_time,
            "max_time": self.max_time,
        }


def measure_time(file, runs) -> RuntimeMeasurementResult:
    print(f"Measuring time for {file}")
    times = []
    for _ in range(runs):
        start_time = time.time()
        subprocess.run(
            [
                "taskset",
                "-c",
                "5",
                "lli",
                file,
            ]
        )
        end_time = time.time()
        times.append(end_time - start_time)
    return RuntimeMeasurementResult(
        sum(times) / runs,
        min(times),
        max(times),
    )


def run_benchmark_evaluation(benchmark_name, run_number):
    results_file_prefix = (
        f"{get_result_folder_name(benchmark_name)}/run_{str(run_number)}"
    )
    subprocess.run(
        ["build/main", f"benchmarks/{benchmark_name}/config.json", results_file_prefix]
    )


def full_evaulation(benchmark_name):
    result_folder = get_result_folder_name(benchmark_name)
    if not os.path.exists(result_folder):
        os.makedirs(result_folder)
    benchmark_folder = f"benchmarks/{benchmark_name}"
    original_file = f"{benchmark_folder}/original.ll"
    opt_file_template = benchmark_folder + "/opt_lvl{}"
    subprocess.run(["opt", "-O3", original_file, "-o", opt_file_template.format(3)])
    subprocess.run(["opt", "-O2", original_file, "-o", opt_file_template.format(2)])
    subprocess.run(["opt", "-O1", original_file, "-o", opt_file_template.format(1)])
    o1_time = measure_time(opt_file_template.format(1), BASELINE_RUNS)
    o2_time = measure_time(opt_file_template.format(2), BASELINE_RUNS)
    o3_time = measure_time(opt_file_template.format(3), BASELINE_RUNS)
    original_time = measure_time(original_file, BASELINE_RUNS)
    # best_time = measure_time(f"{benchmark_folder}/bestfrombig.ll", BASELINE_RUNS)
    print(f"O1 time: {o1_time.time}")
    print(f"O2 time: {o2_time.time}")
    print(f"O3 time: {o3_time.time}")
    # print(f"Best time: {best_time.time}")
    with open(f"{result_folder}/baseline_times.json", "w") as f:
        f.write(
            json.dumps(
                {
                    "o1": o1_time.to_dict(),
                    "o2": o2_time.to_dict(),
                    "o3": o3_time.to_dict(),
                    "original": original_time.to_dict(),
                    # "best": best_time.to_dict(),
                },
                indent=4,
            )
        )
    for run_number in range(RUNS):
        run_benchmark_evaluation(benchmark_name, run_number)
        print(f"Run {run_number} complete")


def plot_results(benchmark_name):
    all_tuner_results = []
    results_folder = get_result_folder_name(benchmark_name)
    csv_files = []
    for filename in os.listdir(results_folder):
        if filename.startswith("run_") and filename.endswith("_results.csv"):
            csv_files.append(filename)
    for csv_file in csv_files:
        all_tuner_results.append([])
        csv_path = os.path.join(results_folder, csv_file)
        df = pd.read_csv(csv_path)
        for index, row in df.iterrows():
            if row["Success"] == 1 and row["Result"] == 0:
                time = row["Average Duration (s)"]
                if not all_tuner_results[-1] or time < all_tuner_results[-1][-1]:
                    all_tuner_results[-1].append(time)
                    continue
            all_tuner_results[-1].append(all_tuner_results[-1][-1])
    zipped_results = list(zip(*all_tuner_results))
    means = [np.mean(x) for x in zipped_results]
    stds = [np.std(x) for x in zipped_results]
    xs = range(len(zipped_results))
    plt.figure(figsize=(20, 10))
    plt.plot(xs, means, label="Beam Search")
    plt.fill_between(
        xs,
        [m - s for m, s in zip(means, stds)],
        [m + s for m, s in zip(means, stds)],
        alpha=0.2,
    )

    with open(f"{results_folder}/baseline_times.json", "r") as f:
        baseline_times = json.load(f)
    plt.plot(xs, [baseline_times["o1"]["time"]] * len(xs), label="O1", linestyle="--")
    plt.plot(xs, [baseline_times["o2"]["time"]] * len(xs), label="O2", linestyle="--")
    plt.plot(xs, [baseline_times["o3"]["time"]] * len(xs), label="O3", linestyle="--")
    plt.plot(
        xs,
        [baseline_times["original"]["time"]] * len(xs),
        label="Original",
        linestyle="--",
    )
    # plt.plot(
    #     xs,
    #     [baseline_times["best"]["time"]] * len(xs),
    #     label="Best",
    #     linestyle="--",
    # )

    plt.legend()
    plt.show()


if __name__ == "__main__":
    full_evaulation(sys.argv[1])
    plot_results(sys.argv[1])
