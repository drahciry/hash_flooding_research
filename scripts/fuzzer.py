import os
import sys
import random
import string
import subprocess

BINARY_PATH = "./fuzzer_target"
MAX_KEY_LENGTH = 1048576
REHASH_STRESS_COUNT = 50000

def generate_random_string(length: int) -> str:
    """"Generates a random alphanumeric string of a specific length."""
    chars = string.ascii_letters + string.digits
    return ''.join(random.choice(chars) for _ in range(length))

def payload_rehash_exhaustion() -> list[str]:
    """
    Vulnerability Vector: Memory Fragmentation & Tail Latency.
    Generates thousands of small unique keys to force the Hash Table
    to hit the LOAD_FACTOR_THRESHOLD repeatedly. This triggers massive
    calloc() operations, heavily fragmenting the OS Heap and causing
    severe O(N) copy latence peaks.
    """
    print(f"[*] Generating {REHASH_STRESS_COUNT} payloads for Rehash Exhaustion...")
    return [generate_random_string(16) for _ in range(REHASH_STRESS_COUNT)]

def payload_cpu_cache_choke() -> list[str]:
    """
    Vulnerability Vector: L1/L2 Cache Eviction & CPU Starvation.
    Generates keys slightly below the MAX_KEY_LENGTH (e.g., 1,048,500 bytes).
    Forces the C program to iterate over 1MB of memory per cw_hash execution,
    evicting useful data from the CPU cache and bottlenecking the memory bus.
    """
    print(f"[*] Generating Max-Length payloads for CPU/Cache Starvation...")
    return [generate_random_string(MAX_KEY_LENGTH - 64) for _ in range(50)]

def payload_edge_cases() -> list[str]:
    """
    Vulnerability Vector: Unsanitized Inputs & Boundary Logic.
    Tests if the C code correctly rejects edge cases without SegFaulting.
    """
    print(f"[*] Generating Edge-Case payloads...")
    return [
        "",
        "A",
        "B" * 5000,
        generate_random_string(MAX_KEY_LENGTH + 100)
    ]

def payload_incremental_expansion() -> list[str]:
    """
    Vulnerability Vector: Cold-Start DoS / Context Switch Exhaustion.
    Forces the hasher to hit the ensure_capacity() doubling repeatedly.
    Each increment triggers OS entropy syscalls (getrandom/BCrypt), creating
    a compounding latency effect.
    """
    print("[*] Generating Incremental Expansion payloads...")
    payloads = []
    current_capacity = 64
    
    while True:
        current_capacity *= 2
        target_size = current_capacity + 1

        if target_size > MAX_KEY_LENGTH:
            break

        payloads.append(generate_random_string(target_size))

    return payloads

def run_fuzzer(payloads: list[str]):
    """
    Spawns the C binary and feeds the adversarial payloads via stdin.
    Captures and analyzes the execution time for each operation.
    """
    if not os.path.exists(BINARY_PATH):
        print(f"[!] Error: Target Binary '{BINARY_PATH} not found.")
        sys.exit(1)

    print(f"[*] Launching target process: {BINARY_PATH}")

    process = subprocess.Popen(
        [BINARY_PATH],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    max_latency = 0.0
    anomalies = 0

    try:
        for i, payload in enumerate(payloads):
            process.stdin.write(payload + "\n")
            process.stdin.flush()

            output = process.stdout.readline().strip()

            if not output:
                print(f"[!] Target crashed or closed stream at payload {i}!")

            parts = output.split(" | ")
            if len(parts) >= 3:
                time_part = parts[2].split(":")[1]
                latency = float(time_part)

                if latency > max_latency:
                    max_latency = latency

                if latency > 5.0:
                    anomalies += 1
                    print(f"[ANOMALY] {output}")

    except BrokenPipeError:
        print("[!] Broken Pipe: The target binary crashed (Segmentation Fault / OOM).")

    finally:
        process.stdin.close()
        process.terminate()
        process.wait()

        print("\n========================================")
        print("          FUZZING CAMPAIGN END          ")
        print("========================================")
        print(f"Total Payloads Sent : {len(payloads)}")
        print(f"Peak Latency (ms)   : {max_latency:.4f}")
        print(f"Structural Anomalies: {anomalies}")
        print("========================================\n")

if __name__ == "__main__":
    # 1. Test Rehash Bounds
    rehash_payloads = payload_rehash_exhaustion()
    run_fuzzer(rehash_payloads)

    # 2. Test CPU Limits
    cpu_payloads = payload_cpu_cache_choke()
    run_fuzzer(cpu_payloads)

    # 3. Test Edge Cases
    edge_payloads = payload_edge_cases()
    run_fuzzer(edge_payloads)

    # 4. Test Cold-Start / Incremental Context Switches
    incremental_payloads = payload_incremental_expansion()
    run_fuzzer(incremental_payloads)
