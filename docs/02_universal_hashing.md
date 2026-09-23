# Phase 2: Carter-Wegman Mitigation (Universal Hashing)

## Breaking Predictability
To defend against Hash Flooding, merely switching to a "better" deterministic hash function is insufficient. The only mathematical way to neutralize the attack vector is to remove predictability entirely. To achieve this, we implemented the **Carter-Wegman algorithm**, introducing the concept of Universal Hashing.

Instead of hardcoding a static hashing function, Universal Hashing dynamically selects a random hash function from a broader family at the exact moment the data structure is initialized in memory.

## The Mathematical Foundation
The Carter-Wegman algorithm operates on the following modular congruence to calculate the index for a given key $k$:

$$ h(k) = ((a \times k + b) \pmod p) \pmod m $$

Where:
* $p$: Is a sufficiently large prime number (we utilize the Mersenne Prime `4294967291`).
* $m$: Is the current size of the hash table (number of buckets).
* $a$ and $b$: Are **randomly generated** integer coefficients.

### Entropy Injection (CSPRNG)
The core strength of this mitigation lies in how the coefficients $a$ and $b$ are generated. In our `carter_wegman.c` implementation, we make direct system calls to the OS-level Cryptographically Secure Pseudo-Random Number Generator (CSPRNG), reading bytes from `/dev/urandom` on Linux or `BCryptGenRandom` on Windows.

Because these values are generated securely at runtime, the internal state of the hash table is completely invisible and unpredictable. An attacker has no way to guess the values of $a$ and $b$, making it mathematically impossible for them to pre-calculate a valid list of colliding keys offline.

## Empirical Validation
When we subjected our Universal Hash Table to the same aggressive fuzzing payloads that easily crippled the baseline implementation, the malicious keys were uniformly dispersed across the bucket array. The execution time remained strictly $O(1)$, proving that the algorithmic DoS vulnerability was entirely neutralized.

## Architectural Trade-offs and Limitations
While Universal Hashing successfully mitigates algorithmic complexity attacks, incorporating runtime entropy and modular arithmetic introduces significant performance costs. Recognizing these trade-offs is critical for evaluating security in production environments:

* **Processor Overhead (CPU Instructions):** The Carter-Wegman formula heavily relies on multiplication and modulo operations (`%`) against a massive 64-bit prime. The processor's Arithmetic Logic Unit (ALU) requires substantially more clock cycles to execute division-based mathematics compared to legacy hashes like `djb2`, which rely exclusively on ultra-fast bitwise shifts (`<<`) and additions. This mathematically dense approach significantly reduces maximum throughput when processing millions of keys per second.
* **Memory Footprint and Long Strings:** Our `CarterWegmanHasher` implementation requires an array of coefficients to process string characters. Because a unique coefficient is mapped to each byte of the key, exceptionally long inputs (e.g., a 5MB malicious HTTP payload) force the application to dynamically allocate new memory on the heap. This dynamic scaling increases memory consumption and risks heap fragmentation, a weakness not present in modern block-based cryptographic algorithms.
* **Operating System Bottlenecks (Syscalls):** Relying on the OS for secure entropy requires transitioning execution from User Space to Kernel Space. System calls (Syscalls) to read from `/dev/urandom` or `BCryptGenRandom` carry a high computational overhead. If the hash table undergoes frequent rehashing or constant expansion of the coefficient array, these repeated Syscalls will introduce severe latency spikes that deterministic algorithms completely avoid.
