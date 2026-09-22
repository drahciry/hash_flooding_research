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