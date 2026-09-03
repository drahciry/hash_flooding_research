# Phase 2: Carter-Wegman Universal Hashing & Fuzzing

To mitigate Hash Flooding, the hashing mechanism must be unpredictable to an external attacker while remaining computationally efficient. This directory implements a 2-independent Carter-Wegman Universal Hash Table.

By introducing random coefficients generated via Cryptographically Secure PRNGs (`BCryptGenRandom` on Windows, `getrandom` on Linux), the mathematical equation that maps keys to buckets changes completely on every execution. It is mathematically impossible for an attacker to precompute collisions without memory access to the coefficients.

## Contents
* **`carter_wegman.c`**: The mathematical core demonstrating the Universal Hashing algorithm.
* **`hash_table_cw.c`**: The complete, secure hash table implementation using Separate Chaining and graceful degradation logic.
* **`fuzzer.py` & `fuzzer_target.c`**: A black-box testing environment. The Python fuzzer spawns the C binary as a child process, feeding it extreme edge-cases (max-length keys, empty strings, rapid capacity exhaustion) to hunt for segmentation faults, integer overflows, and tail latency spikes.

## Fuzzing the Target
Compile the target harness and launch the Python orchestrator:
```bash
gcc -O2 fuzzer_target.c -o fuzzer_target
python3 fuzzer.py