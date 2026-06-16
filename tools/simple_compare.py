import math
import os
import random
import subprocess
import sys
import tempfile
import time

N = 50


def compile_to_binary(ll_file: str, out_name: str, tmp_dir: str) -> str:
    """Compile an LLVM IR file to a native binary with no extra optimizations."""
    obj_file = os.path.join(tmp_dir, f"{out_name}.o")
    bin_file = os.path.join(tmp_dir, out_name)

    ret = subprocess.call(["llc", "-O0", "-filetype=obj", ll_file, "-o", obj_file])
    if ret != 0:
        print(f"ERROR: llc failed for {ll_file}")
        sys.exit(1)

    ret = subprocess.call(["clang++", "-O0", "-no-pie", obj_file, "-o", bin_file, "-lm"])
    if ret != 0:
        print(f"ERROR: clang++ linking failed for {ll_file}")
        sys.exit(1)

    return bin_file


def apply_opt(ll_file: str, opt_level: str, out_name: str, tmp_dir: str) -> str:
    """Apply opt optimization passes to an LLVM IR file, returning the output .ll path."""
    out_ll = os.path.join(tmp_dir, f"{out_name}.ll")
    ret = subprocess.call(["opt", f"-{opt_level}", ll_file, "-S", "-o", out_ll])
    if ret != 0:
        print(f"ERROR: opt -{opt_level} failed for {ll_file}")
        sys.exit(1)
    return out_ll


def _unique_label(path: str, used: set[str]) -> str:
    base = os.path.splitext(os.path.basename(path))[0] or "tuned"
    label = base
    n = 2
    while label in used:
        label = f"{base}_{n}"
        n += 1
    used.add(label)
    return label


def main(original_ll: str, tuned_lls: list[str]):
    with tempfile.TemporaryDirectory() as tmp_dir:
        variants: dict[str, str] = {}
        label_paths: dict[str, str] = {}

        print("Compiling original ...")
        variants["original"] = compile_to_binary(original_ll, "original", tmp_dir)
        label_paths["original"] = original_ll

        used_labels: set[str] = {"original", "O1", "O2", "O3"}
        for path in tuned_lls:
            label = _unique_label(path, used_labels)
            print(f"Compiling tuned ({label}) ...  {path}")
            variants[label] = compile_to_binary(path, label.replace(os.sep, "_"), tmp_dir)
            label_paths[label] = path

        for level in ("O1", "O2", "O3"):
            print(f"Optimizing and compiling -{level} ...")
            opt_ll = apply_opt(original_ll, level, level, tmp_dir)
            variants[level] = compile_to_binary(opt_ll, level, tmp_dir)
            label_paths[level] = f"{original_ll} (opt -{level})"

        print()

        labels = list(variants.keys())
        times = {label: [] for label in labels}
        schedule = labels * N
        random.shuffle(schedule)

        for label in schedule:
            binary = variants[label]
            print(f"Running {label}")
            start = time.time()
            subprocess.call(["taskset", "-c", "5", binary])
            end = time.time()
            times[label].append(end - start)

        w = max(8, max(len(l) for l in labels))
        print()
        for label in labels:
            avg = sum(times[label]) / N
            stddev = math.sqrt(sum((t - avg) ** 2 for t in times[label]) / N)
            path_note = label_paths.get(label, "")
            if path_note and label not in ("O1", "O2", "O3"):
                print(f"{label:{w}s}: {avg:.4f}s avg  ±{stddev:.4f}s  ({path_note})")
            else:
                print(f"{label:{w}s}: {avg:.4f}s avg  ±{stddev:.4f}s")


if __name__ == "__main__":
    if len(sys.argv) < 3:
        prog = os.path.basename(sys.argv[0])
        print(
            f"Usage: {prog} <original.ll> <tuned.ll> [more_tuned.ll ...]\n"
            "  Compares original, every tuned .ll you pass, and opt -O1/-O2/-O3 on original."
        )
        sys.exit(1)
    main(sys.argv[1], sys.argv[2:])
