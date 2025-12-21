# Structural Autotuning for LLVM IR Code

**A Research Prototype for Exploring Semantic-Agnostic Code Mutations**

## Overview

Optimizing programs for performance is a complex and time-consuming task. Traditional compilers rely on predefined optimization passes that apply semantic-preserving transformations to find and transform known patterns. While effective, these approaches are limited by their reliance on expert-defined patterns and often require manual fine-tuning.

This project explores a novel direction: **structural autotuning through low-level, sub-pass mutations at the LLVM IR level**. Rather than optimizing the selection and order of predefined compiler passes, we investigate semantic-agnostic mutations that can discover novel high-performance variants by exploring a vastly larger search space.

### Key Contributions

1. **Framework for Structural Mutations**: A system that generates structurally mutated LLVM IR variants through low-level transformations
2. **Autotuning Integration**: Beam search exploration strategy to navigate the expanded transformation space
3. **Performance Evaluation**: Comparison against traditional compiler optimization techniques demonstrating the potential for discovering optimizations beyond standard compiler passes

## Project Goals

The primary objectives of this work are threefold:

1. **Develop a mutation framework** that enables systematic generation of structurally mutated LLVM IR variants
2. **Integrate autotuning processes** that can identify high-performing variants tailored to specific hardware and workloads
3. **Evaluate effectiveness** by comparing performance outcomes against traditional compiler optimization techniques

## Structural Mutations

The framework implements five types of structural mutations that operate at the LLVM IR level. These mutations are designed as "micromutations" - low-level transformations that can be applied independently or in combination to explore the optimization space. Each mutation is parameterized by a decision vector that controls its specific behavior (e.g., which function, basic block, or instruction to target), allowing for systematic exploration of the mutation space.

### 1. ADD_RANDOM_ARITHMETIC

This mutation selects two arithmetic instructions from the same basic block and inserts a new arithmetic operation between them. The new operation uses the results of the two selected instructions as operands, and replaces some uses of the second instruction with the result of the new operation.

**How it works:**
1. Selects a function and basic block
2. Finds arithmetic instructions with numeric return types (integer or floating point)
3. Selects two instructions (Inst1 and Inst2) where Inst1 appears before Inst2
4. Creates a random arithmetic operation (Add, Sub, Mul, UDiv, SDiv) between Inst1 and Inst2
5. Handles type conversions if needed (integer casts, int-to-float, float casts)
6. Inserts the new instruction after Inst2
7. Replaces uses of Inst2 (except the first use) with the new instruction's result

**Example transformation:**

**Before:**
```llvm
define i32 @example(i32 %a, i32 %b) {
entry:
  %1 = mul i32 %a, 5
  %2 = add i32 %b, 10
  %3 = sub i32 %2, 3
  ret i32 %3
}
```

**After (with ADD_RANDOM_ARITHMETIC selecting %1 and %2, creating an add operation):**
```llvm
define i32 @example(i32 %a, i32 %b) {
entry:
  %1 = mul i32 %a, 5
  %2 = add i32 %b, 10
  %random_add_12345 = add i32 %1, %2  ; New instruction inserted after %2
  %3 = sub i32 %random_add_12345, 3  ; %3 now uses new instruction instead of %2
  ret i32 %3
}
```

Note: The new instruction uses both %1 and %2 as operands. The first use of %2 (in the new instruction itself) is preserved, but subsequent uses (like in %3) are replaced with the new instruction's result.

### 2. MOVE_BLOCKWISE

This mutation moves a contiguous sequence of instructions within a basic block to a different position in the same basic block, without changing the relative order of the moved instructions.

**How it works:**
1. Selects a function and basic block
2. Selects three instructions: First, Last (defining a range), and InsertBefore (insertion point)
3. The insertion point must be outside the range [First, Last]
4. Uses `BasicBlock::splice()` to move the instruction sequence

**Example transformation:**

**Before:**
```llvm
define i32 @example(i32 %a, i32 %b) {
entry:
  %1 = add i32 %a, 1
  %2 = mul i32 %b, 2
  %3 = sub i32 %1, %2
  %4 = add i32 %a, 10
  ret i32 %3
}
```

**After (with MOVE_BLOCKWISE moving instructions %2 and %3 to after %4):**
```llvm
define i32 @example(i32 %a, i32 %b) {
entry:
  %1 = add i32 %a, 1
  %4 = add i32 %a, 10
  %2 = mul i32 %b, 2
  %3 = sub i32 %1, %2
  ret i32 %3
}
```

Note: This mutation moves instructions structurally without checking data dependencies. The resulting IR may be invalid if moved instructions depend on instructions that come after them in the new order, or if instructions that depend on the moved instructions come before them. The mutation will fail validation if the resulting IR is invalid.

### 3. ADD_NEW_COND

This mutation adds a new conditional branch by comparing two existing register values and creating a new basic block that branches based on the comparison result.

**How it works:**
1. Selects a function and basic block
2. Finds the terminator instruction of the selected basic block
3. Collects all used registers from the function
4. Selects two random registers to compare
5. Creates a comparison (ICmpEQ for integers/pointers, FCmpOEQ for floats)
6. Handles type conversions if the registers have different types
7. Creates a new basic block that branches to the original successor
8. Replaces the original terminator with a conditional branch: `br i1 %comparison, label %newblock, label %original_successor`

**Example transformation:**

**Before:**
```llvm
define i32 @example(i32 %a, i32 %b) {
entry:
  %1 = add i32 %a, %b
  %2 = mul i32 %1, 2
  br label %next

next:
  %3 = add i32 %2, 5
  ret i32 %3
}
```

**After (with ADD_NEW_COND comparing %1 and %2):**
```llvm
define i32 @example(i32 %a, i32 %b) {
entry:
  %1 = add i32 %a, %b
  %2 = mul i32 %1, 2
  %regCompare = icmp eq i32 %1, %2
  br i1 %regCompare, label %newblock123456, label %next

newblock123456:
  br label %next

next:
  %3 = add i32 %2, 5
  ret i32 %3
}
```

### 4. UNSAFE_MEM_2_REG

This mutation promotes a stack-allocated variable (alloca) to an SSA register by eliminating the alloca, load, and store instructions and using phi nodes where necessary. It uses LLVM's `PromoteMemToReg` function with a simplified promotability check (`simpleIsAllocaPromotable`) that allows:
- Load instructions that read from the alloca
- Store instructions where the alloca is the pointer operand
- GetElementPtr and BitCast instructions (which are recursively checked)

Any other use types cause the promotion to be rejected. This is more permissive than LLVM's standard `mem2reg` pass, which performs more conservative safety analysis.

**How it works:**
1. Selects a function and basic block
2. Finds alloca instructions in the selected basic block
3. Selects one alloca to promote
4. Checks if the alloca is promotable using `simpleIsAllocaPromotable()` (allows Load, Store, GetElementPtr, and BitCast uses)
5. Creates a DominatorTree for the function
6. Calls `PromoteMemToReg()` to perform the promotion

**Example transformation:**

**Before:**
```llvm
define i32 @example(i32 %a) {
entry:
  %x = alloca i32, align 4
  store i32 %a, ptr %x, align 4
  %1 = load i32, ptr %x, align 4
  %2 = add i32 %1, 5
  ret i32 %2
}
```

**After (with UNSAFE_MEM_2_REG promoting %x):**
```llvm
define i32 @example(i32 %a) {
entry:
  %2 = add i32 %a, 5
  ret i32 %2
}
```

The alloca, store, and load are eliminated, and the value flows directly through SSA registers. In more complex cases with multiple stores, phi nodes would be inserted at control flow merge points.

### 5. DELETE_RANDOM_INSTRUCTION

This mutation removes a random instruction from a basic block, but only if the instruction has no uses (i.e., its result is not referenced by any other instruction).

**How it works:**
1. Selects a function and basic block
2. Collects all non-terminator instructions from the basic block
3. Selects a random instruction
4. Checks if the instruction has no uses (`use_empty()`)
5. If unused, deletes the instruction using `eraseFromParent()`

**Example transformation:**

**Before:**
```llvm
define i32 @example(i32 %a, i32 %b) {
entry:
  %1 = add i32 %a, 1
  %2 = mul i32 %b, 2
  %3 = add i32 %1, %2
  %4 = mul i32 %1, 5  ; Unused result
  ret i32 %3
}
```

**After (with DELETE_RANDOM_INSTRUCTION removing the unused %4):**
```llvm
define i32 @example(i32 %a, i32 %b) {
entry:
  %1 = add i32 %a, 1
  %2 = mul i32 %b, 2
  %3 = add i32 %1, %2
  ret i32 %3
}
```

## Autotuning Strategy

The framework employs a **beam search** algorithm to explore mutation sequences while maintaining correctness:

- **Search Tree Structure**: Each node represents a mutation sequence, with children representing extensions of that sequence. The tree is built incrementally as promising paths are discovered.

- **Scoring Function**: The algorithm uses a composite scoring function that balances correctness and performance. Correct variants receive higher scores (0.8-1.0 range), with faster variants scoring higher. Incorrect variants are penalized (0.0-0.2 range) but still explored to maintain diversity in the search space.

- **Exploration Strategy**: The algorithm probabilistically selects paths through the search tree based on node scores, balancing exploitation of known good paths with exploration of new branches. The search maintains up to 400 mutations per path and operates within a budget of 1000 program executions.

## Correctness Validation

The system employs a two-stage validation process:

1. **Execution Validation**: Ensures the program runs successfully (6-second timeout, multiple repetitions for statistical reliability)

2. **Output Correctness**: Type-specific distance metrics compute the deviation from expected output:
   - **Scalar**: Absolute difference from expected value
   - **Array**: Sum of element-wise absolute differences
   - **Matrix2D**: Sum of element-wise absolute differences across all dimensions

Programs with `result == 0` (exact match) are considered correct. Only mutations that pass both validation stages are added to the search tree.

## Results and Evaluation

The framework has been evaluated on benchmark programs, comparing the performance of autotuned variants against traditional compiler optimizations (O1, O2, O3) and the original unoptimized code.

### Monte Carlo Pi Calculation Benchmark

![Pi Benchmark Comparison](diagrams/pi_benchmark_comparison.png)

The benchmark implements a Monte Carlo method for calculating π by generating random points in a unit square and determining the ratio that fall within a unit circle. This benchmark is based on the Monte Carlo pi computation from the [LLVM benchmark repository maintained by Microsoft](https://github.com/microsoft/checkedc-llvm-test-suite/blob/master/SingleSource/Benchmarks/Misc/pi.c). This benchmark demonstrates significant performance improvements through structural autotuning:

- **Original runtime**: 0.1132s (baseline)
- **Compiler optimizations**: 
  - O1: 0.1090s
  - O2: 0.1082s  
  - O3: 0.1087s
- **Best autotuned variant**: Achieves performance below all compiler optimization levels
- The beam search algorithm progressively improves performance over ~200 iterations, eventually outperforming traditional compiler optimizations
- Multiple correct variants were discovered, with the search process exploring over 300 correct mutation paths

### Key Observations

1. **Progressive Improvement**: The beam search algorithm shows a clear downward trend in execution time as it explores the mutation space, indicating effective search strategy
2. **Outperforming Compiler Optimizations**: Autotuned variants achieve better performance than standard compiler optimization levels (O1-O3)
3. **Correctness Maintenance**: All reported variants maintain program correctness (result = 0), validating the correctness validation system
4. **Search Space Exploration**: The framework successfully navigates a large search space while maintaining correctness constraints

## Building and Usage

### Requirements

- **LLVM** (with development headers and libraries)
- **CMake** (version 3.10 or higher)
- **C++17** compatible compiler
- **Python 3** with packages: `matplotlib`, `numpy`, `pandas`, `pyvis`

### Building

```bash
./scripts/build.sh
```

Or manually:
```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

### Running Autotuning

```bash
./build/main <config_file> <results_file_prefix>
```

**Example:**
```bash
./build/main benchmarks/pi/config.json eval_results/pi/run_0
```

This generates:
- `<prefix>_results.csv` - Execution statistics for each mutation attempt
- `<prefix>_tree.json` - Complete beam search tree structure

### Configuration File Format

```json
{
    "original": "path/to/original.ll",
    "modified": "path/to/modified.ll",
    "correct_output": "path/to/correct_output.json",
    "output_file": "path/to/output.json",
    "output_type": "scalar"
}
```

**Output Types:** `"scalar"`, `"array"`, or `"matrix2d"`

## Technical Details

### Runtime Measurement

- Uses LLVM's `lli` interpreter for execution
- CPU affinity pinning (`taskset -c 5`) for consistent measurements to isolate CPU core 5
- Multiple repetitions (default: 2) for statistical reliability
- 6-second timeout to prevent hanging executions
