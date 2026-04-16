import itertools

def gerar_colisoes():
    blocos_base = ["bY", "az"]
    combinacoes = itertools.product(blocos_base, repeat=16)

    with open("colisoes.txt", "w") as arquivo:
        for pedaco in combinacoes:
            palavra = "".join(pedaco)
            arquivo.write(palavra + "\n")

    print("Arquivo 'colisoes.txt' gerado com 65.536 strings maliciosas!")

if __name__ == "__main__":
    gerar_colisoes()