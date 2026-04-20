import os
import sys
import time

N = 20


def main(file1, file2):
    times1 = []
    times2 = []
    for _ in range(N):
        start = time.time()
        print(f"Running {file1}")
        os.system(f"taskset -c 5 lli {file1}")
        end = time.time()
        times1.append(end - start)
        start = time.time()
        print(f"Running {file2}")
        os.system(f"taskset -c 5 lli {file2}")
        end = time.time()
        times2.append(end - start)

    print(f"{file1}: {sum(times1) / N}")
    print(f"{file2}: {sum(times2) / N}")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
