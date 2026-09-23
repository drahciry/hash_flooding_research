import itertools

FILENAME = "./results/collisions.txt"

def gerar_colisoes(file_name: str = FILENAME) -> None:
    file_name = FILENAME
    base_keys = ["bY", "az"]
    combinations = itertools.product(base_keys, repeat=16)

    with open(file_name, "w") as file:
        for combination in combinations:
            word = "".join(combination)
            file.write(word + "\n")

    print(f"[*] File '{file_name}' generated with 65.536 mallicious strings!")

if __name__ == "__main__":
    gerar_colisoes()