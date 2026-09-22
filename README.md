# Hash Flooding Attack and Universal Hashing Mitigation

## Overview
This repository contains an architectural research project and Proof of Concept (PoC) focused on **Hash Flooding**, a classic Denial of Service (DoS) vulnerability. The project explores how deterministic hash table implementations can be maliciously manipulated to degrade algorithmic performance from $O(1)$ to $O(n)$, causing severe CPU exhaustion.

After establishing a vulnerable baseline using Double Hashing, the research implements a robust cryptographic mitigation using **Carter-Wegman Universal Hashing**, effectively shielding the data structure against predictable collision generation.

## Repository Architecture
To ensure memory safety and maintain a modular, production-ready codebase, the repository is strictly divided:

* `include/`: Public API headers. Utilizes opaque pointers to enforce strict memory encapsulation and hide internal cryptographic states.
* `src/`: Core C implementations of the algorithms, memory management, and entropy generation.
* `docs/`: In-depth theoretical research, mathematical modeling, and memory auditing logs.
* `tests/`: Fuzzing harnesses and memory stress-testing endpoints designed for Valgrind profiling.
* `scripts/`: Python-based offensive tooling for payload generation and automated fuzzing orchestration.

## Build and Execution Instructions
This project utilizes a `Makefile` to automate compilation with strict security flags (`-Wall -Wextra -g3 -O0`).

1. **Build all compilation targets:**
   ```bash
   make all
   ```
2. **Execute the fuzzer against the mitigated target:**
   ```bash
   ./bin/fuzzer < results/collisions.txt
   ```
3. **Clean build artifacts:**
   ```bash
   make clean
   ```

## Detailed Documentation
For a deep dive into the mathematical foundation and technical engineering of each research phase, please refer to the detailed reports located in the `docs/` directory.