# Hash Flooding Attack: Research & Mitigation

An in-depth algorithmic research project exploring Denial of Service (DoS) attacks via Hash Flooding. This repository demonstrates how deterministic hash functions can be exploited to force $O(N)$ lookup times in Hash Tables, and implements a robust, production-ready defense using 2-independent Carter-Wegman Universal Hashing.

## 🎯 Research Objectives
- **Offensive Analysis:** Exploit predictable hash functions (like DJB2 or standard modulo approaches) by generating massive amounts of colliding keys.
- **Architectural Impact:** Measure how hash collisions degrade CPU performance, exhaust L1/L2 caches, and fragment the OS Heap.
- **Defensive Engineering:** Implement a mathematically proven Universal Hashing strategy in C, backed by OS-level Cryptographically Secure Pseudo-Random Number Generators (CSPRNG) like `/dev/urandom` and Windows CNG.
- **Fuzzing & Stress Testing:** Build Python-based black-box fuzzers to stress the C implementations via `stdin`/`stdout` and profile memory resilience.

## 📂 Repository Structure

* `start_of_research/` — Initial phase. Contains vulnerable C hash table implementations and the Python scripts used to reverse-engineer the hash and generate collision payloads.
* `universal_hashing/` — Defensive phase. Contains the secure Carter-Wegman implementation and the adversarial black-box fuzzer to test its resilience.
* `valgrind_tests/` — Profiling phase. Memory management validation, leak detection, and cache-miss analysis using Valgrind.

## 🛠️ Tech Stack
- **C:** Low-level data structures, pointer arithmetic, memory management, and OS system calls.
- **Python:** Black-box fuzzing, collision payload generation, and test orchestration.
- **Valgrind:** Memory profiling and memory leak detection.