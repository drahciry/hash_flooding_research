# Hash Flooding Research & Mitigation

This repository contains research, proofs-of-concept, and mitigation strategies regarding **Hash Flooding** (also known as Algorithmic Complexity Attacks). Hash flooding is a type of Denial of Service (DoS) attack where a malicious actor intentionally feeds data to an application that results in worst-case time complexity (from $O(1)$ to $O(N)$ complexity) in hash table operations, exhausting CPU resources.

## Repository Structure

The project is divided into distinct phases of research and testing:

*   **`start_of_research/`**: Contains the initial implementations and scripts used to demonstrate the vulnerability. This includes collision generators (`generate_collisions.py`) and basic hash table implementations (`hash_table_dh.c`, `hash_table_dh_old.c`) susceptible to flooding attacks.
*   **`universal_hashing/`**: Focuses on the core mitigation strategy utilizing Carter-Wegman universal hashing to introduce randomness into the hash function family, thereby rendering deterministic collision generation mathematically infeasible for an attacker.
*   **`valgrind_tests/`**: Contains memory management validation and profiling tools to ensure the integrity of the implementations and guarantee the absence of memory leaks during high-load operations.

## Security & Threat Model Disclaimer

**Important Note on Testing Utilities:** 
This repository is a security research project. The core focus is strictly on the algorithmic complexity and resilience of the hash table data structures. 

Certain files within this repository serve purely as local testing harnesses, fuzzers, and collision generation utilities (e.g., Python scripts and I/O reading blocks in C). These utilities may use standard input parsing functions (like `fgets`) to read testing payloads or collision text files (`collisions.txt`). 

These I/O operations are **strictly out of scope** for the threat model of the hash table implementation itself. Any static analysis or vulnerability flags raised on standard I/O parsing within the testing environment do not reflect the security posture of the actual hash table algorithms being researched. The mitigation provided (such as Universal Hashing) operates entirely at the data structure level.

## Getting Started

To explore the research, it is recommended to start in the `start_of_research/` directory to understand how standard hash functions can be exploited, and then move to the `universal_hashing/` directory to analyze the Carter-Wegman mitigation.
