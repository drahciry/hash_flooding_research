# Phase 1: Vulnerability Exploration (Hash Flooding)

## The Problem with Determinism
A Hash Table is designed to store and retrieve data with a constant time complexity, denoted as $O(1)$. To achieve this, a hash function is used to map a given key (such as a string) to a numeric index within an array (the buckets).

In this initial phase, we implemented a baseline table utilizing the `djb2` hash function and Double Hashing for collision resolution. The fundamental flaw of `djb2` (and similar legacy algorithms like MurmurHash3 or CityHash) is that it is **strictly deterministic and lacks any secret internal state**.

Because the algorithm is entirely predictable, if an attacker knows the table size ($m$), they can execute an offline script to compute exactly which keys will yield the same mathematical index.

## The Denial of Service (DoS) Vector
A Hash Flooding attack exploits this mathematical predictability. The attacker floods the target application (e.g., via HTTP POST parameters or JSON payloads) with thousands of carefully crafted keys designed to collide at the exact same bucket.

### The Memory Cascade Effect
When massive collisions occur, the hash table completely loses its $O(1)$ efficiency.
1. The CPU attempts to insert the data but finds the initial index occupied.
2. The collision resolution mechanism (Double Hashing) calculates the next probe sequence.
3. The next index is also occupied due to the clustered attack.
4. The CPU enters an exhaustive loop, scanning memory arrays sequentially.

The time complexity degrades silently to $O(n)$ per insertion. A server that typically processes thousands of requests in milliseconds will suddenly take minutes or hours, exhausting CPU cycles and resulting in a total Denial of Service. Our Python tools in the `scripts/` directory empirically prove this exponential degradation.