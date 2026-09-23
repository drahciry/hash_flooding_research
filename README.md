# Hash Flooding Vulnerability Research

## Overview
This repository contains an architectural research project and Proof of Concept (PoC) focused on **Hash Flooding**, an algorithmic complexity vulnerability that leads to Denial of Service (DoS). The research investigates how deterministic hash table implementations can be maliciously manipulated to degrade algorithmic performance from $O(1)$ to $O(n)$, causing severe CPU exhaustion.

The core objective of this project is to analyze the mechanics of the vulnerability using a baseline implementation (Double Hashing with `djb2`), and subsequently explore and implement cryptographic mitigations. Current mitigation research includes **Carter-Wegman Universal Hashing**, with future iterations planned to incorporate **SipHash**.

## Repository Architecture
To ensure memory safety and maintain a modular, production-ready codebase, the repository is strictly divided:

* `include/`: Public API headers. Utilizes opaque pointers to enforce strict memory encapsulation and hide internal cryptographic states.
* `src/`: Core C implementations of the hash tables, mitigation algorithms, memory management, and entropy generation.
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
   python3 -u ./scripts/fuzzer.py > ./results/fuzzer.log
   ```
3. **Clean build artifacts:**
   ```bash
   make clean
   ```

## Detailed Documentation
For a deep dive into the mathematical foundation and technical engineering of each research phase, please refer to the detailed reports located in the `docs/` directory.

## References & Bibliography
The theoretical foundation and vulnerability mechanics explored in this repository are based on the following publications and resources:

1. **O. Yigit.** "Hash Functions." York University. http://www.cse.yorku.ca/~oz/hash.html (accessed Apr. 2026).
2. **Crosby, S. A., & Wallach, Dan S. (2003).** *Denial of Service via Algorithmic Complexity Attacks*. USENIX Security Symposium.
3. **Carter, J. L., & Wegman, M. N. (1979).** *Universal Classes of Hash Functions*. Journal of Computer and System Sciences, 18(2), 143-154.

*(Future reading planned: Aumasson, J.-P., & Bernstein, D. J. (2012). SipHash: a fast short-input PRF. Progress in Cryptology – INDOCRYPT 2012.)*

## Acknowledgments & Authorship
The core architecture, algorithms, and security implementations in this repository were authored entirely by me. Large Language Models (Google Gemini) were utilized strictly as writing assistants to format, standardize, and translate the Doxygen code comments and Markdown documentation into professional English.
