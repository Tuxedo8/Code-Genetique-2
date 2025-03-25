***NOTE DE L'AUTEUR (Saphir Gobbi): Les fichiers code.c et code.h ont été modifié afin de complété l'activité.***

# Sujet de l'activité
Dans cette partie de l'activité, vous devez écrire les mêmes fonctions que dans l'activité précédente mais sans créer de nouveaux codes dans les opérations. À la place, vous devrez utiliser un des codes passés en paramètre en le redimensionnant au besoin.

## Modification a faire

Vous ne modifierez uniquement les fichiers **code.c** et **code.h**.

Plus précisément, cela concerne les fonctions *code_ajoute* (qui était déjà ainsi dans la première partie), *coupe_code* (qui doit maintenant modifier a et le placer dans ***b***, tandis que ***c*** reste un nouveau code à créer) et *combine_codes* (qui doit maintenant modifier a et le renvoyer). Ces modifications d'un code passé en paramètre devront se faire en le redimensionnant au besoin.

### En outre, dans cette partie, il y a deux différences concernant la mémoire :


- Il faudra maintenant stocker les codons de manière compacte : comme il y a 4 codons différents, 2 bits suffisent à stocker un codon. Il vous faudra donc stocker 4 codons par octet que vous allouerez.

- Il n'y a plus de limite de taille : vos codes devront grossir sans limite, du moment que les allocations mémoire réussissent. En cas d'erreur d'allocation, le comportement des fonctions devra être similaire à ce qui était demandé dans la partie précédente : *alloue_code* devra renvoyer NULL, tandis que *code_ajoute*, *coupe_code* et *combine_codes* devront laisser inchangés les codes qui leur sont passés en paramètre.

### Pour tester les allocations votre code :
- ``make test_Q1`` suivis de ``./test_Q1``
- ``make test_Q2`` suivis de ``./test_Q2``
