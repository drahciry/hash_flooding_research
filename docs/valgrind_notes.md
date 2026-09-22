# Memory Safety Auditing and Valgrind Profiling

## The Danger of Introducing New Vulnerabilities
When evolving the architecture to support Universal Hashing, we transitioned the collision resolution mechanism from Open Addressing (Double Hashing) to Separate Chaining (Linked Lists). This architectural shift required aggressive dynamic memory allocation for internal nodes and expandable arrays to store the Carter-Wegman coefficients.

In C programming, improper management of the heap can introduce critical vulnerabilities that rival the original DoS attack:
* **Memory Leaks:** Slow exhaustion of the heap, eventually causing the server process to crash.
* **Use-After-Free / Double-Free:** Incorrect pointer deallocation, which an attacker can exploit to corrupt execution flow or achieve arbitrary code execution.

## Fortification via Opaque Pointers
To mitigate unauthorized memory manipulation by external modules, our entire public API (`include/`) strictly enforces the use of **Opaque Pointers**. The actual data structures (`struct HashTable` and `struct Node`) are defined exclusively within the `.c` source files.

API consumers can create and destroy the table using dedicated lifecycle functions (`ht_create` and `ht_destroy`), but they are syntactically blocked from reading or modifying internal pointers, bucket arrays, or the cryptographic state.

## Valgrind Memcheck Validation
To guarantee the integrity of the memory lifecycle, we engineered isolated test harnesses in the `tests/memory/` directory. These harnesses perform extreme stress tests:
1. **Stress Allocation:** Insertion of tens of thousands of unique keys, forcing the `HashTable` to trigger `rehash()` operations and reallocate arrays multiple times.
2. **Aggressive Deallocation:** Arbitrary deletion of thousands of keys to test `node_destroy` logic and ensure linked list tombstones are safely unlinked.
3. **Teardown:** Invocation of `ht_destroy()` to dismantle the entire topology.

By profiling the compiled binaries using **Valgrind (Memcheck)**, we achieved a verified report of `0 bytes definitely lost` and `0 invalid reads/writes`. This audit confirms that every `malloc`, `calloc`, and `strdup` call is perfectly paired with a corresponding `free`, resulting in a data structure that is both mathematically resilient and memory-safe at the processor level.