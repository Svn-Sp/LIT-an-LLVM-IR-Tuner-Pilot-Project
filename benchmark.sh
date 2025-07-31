#!/bin/bash

echo "Benchmarking original.ll..."
echo "Run times for original.ll:"
for i in {1..10}
do
    time=$(taskset -c 5 /usr/bin/time -f "%e" lli original.ll 2>&1)
    echo "Run $i: $time seconds"
done

echo -e "\nOptimizing original.ll..."
opt -O3 original.ll -o optimized.ll

echo -e "\nBenchmarking optimized.ll..."
echo "Run times for optimized.ll:"
for i in {1..10}
do
    time=$(taskset -c 5 /usr/bin/time -f "%e" lli optimized.ll 2>&1)
    echo "Run $i: $time seconds"
done 


echo -e "\nBenchmarking best.ll..."
echo "Run times for best.ll:"
for i in {1..10}
do
    time=$(taskset -c 5 /usr/bin/time -f "%e" lli best.ll 2>&1)
    echo "Run $i: $time seconds"
done