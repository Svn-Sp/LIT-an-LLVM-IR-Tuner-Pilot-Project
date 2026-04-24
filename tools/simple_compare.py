import math
import os
import random
import subprocess
import sys
import tempfile
import time

N = 100


def compile_to_binary(ll_file: str, tmp_dir: str) -> str:
    """Compile an LLVM IR file to a native binary with no extra optimizations."""
    base = os.path.splitext(os.path.basename(ll_file))[0]
    obj_file = os.path.join(tmp_dir, f"{base}.o")
    bin_file = os.path.join(tmp_dir, base)

    # Compile IR → object file (no optimization passes)
    ret = subprocess.call(["llc", "-O0", "-filetype=obj", ll_file, "-o", obj_file])
    if ret != 0:
        print(f"ERROR: llc failed for {ll_file}")
        sys.exit(1)

    # Link object → executable (no optimization, just linking)
    ret = subprocess.call(["clang++", "-O0", "-no-pie", obj_file, "-o", bin_file, "-lm"])
    if ret != 0:
        print(f"ERROR: clang++ linking failed for {ll_file}")
        sys.exit(1)

    return bin_file


def main(files):
    with tempfile.TemporaryDirectory() as tmp_dir:
        # Compile all files upfront
        binaries = {}
        for f in files:
            print(f"Compiling {f} ...")
            binaries[f] = compile_to_binary(f, tmp_dir)
        print()

        times = {f: [] for f in files}
        schedule = files * N
        random.shuffle(schedule)

        for f in schedule:
            binary = binaries[f]
            print(f"Running {f}")
            start = time.time()
            subprocess.call(["taskset", "-c", "5", binary])
            end = time.time()
            times[f].append(end - start)

        print()
        for f in files:
            avg = sum(times[f]) / N
            stddev = math.sqrt(sum((t - avg) ** 2 for t in times[f]) / N)
            print(f"{f}: {avg:.4f}s avg  ±{stddev:.4f}s")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <file1> [file2 ...]")
        sys.exit(1)
    main(sys.argv[1:])
