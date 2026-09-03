# Memory Profiling & Valgrind Analysis

In low-level systems programming, mitigating an algorithmic DoS is only half the battle; the structure must also survive extreme memory pressure without leaking or corrupting the heap.

This directory focuses on memory safety and architectural profiling. Using Valgrind, the implementations are audited for:
- **Memory Leaks:** Ensuring every `malloc` and `strdup` has a corresponding `free` during deep destructions (`ht_destroy`).
- **Uninitialized Values:** Validating that entropy generation (`generate_secure_uint64`) correctly fills 64-bit registers before arithmetic operations.
- **Cache Miss Profiling (Optional via Cachegrind):** Analyzing the impact of Separate Chaining node scattering on the L1/L2 CPU caches.

## Running the Checks
```bash
gcc -g hash_table_cw.c -o hash_table_memcheck
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./hash_table_memcheck