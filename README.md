# Analyse de fichiers WAV en C

Projet individuel du cours de systèmes embarqués (L2 EEA, Université Côte d'Azur). Il s'agit de retrouver des messages cachés dans un fichier WAV en lisant directement ses octets.

## Les outils

**`detecter_fin_wav`** compare la taille du fichier à celle annoncée par son en-tête, puis affiche en hexadécimal et en ASCII tout ce qui se trouve après les données audio.

**`comparer_wav_xor`** compare un WAV original et sa version modifiée par XOR. Il rassemble les bits de différence pour reconstituer un message, et peut ensuite déchiffrer un texte en César (décalage de 5).

## Utilisation

```
make
build/detecter_fin_wav fichier.wav
build/comparer_wav_xor original.wav modifie.wav "texte_chiffre"
```

## Tests

```
make test
```

Les tests génèrent de petits WAV synthétiques, compilés avec AddressSanitizer et UBSan. Les fichiers audio du sujet ne sont pas dans ce dépôt, car ils viennent d'un film.

## Limites connues

- L'en-tête est supposé faire exactement 44 octets.
- Les fichiers du sujet sont en 8 bits mono, alors que l'outil XOR lit les données par groupes de 2 octets : il n'examine donc qu'un octet sur deux.
- Le déchiffrement César traite les lettres et les chiffres à part, avec quelques cas particuliers ajoutés à la main. Un décalage de 5 sur tout le code ASCII serait plus simple : `&` donnerait `!`, alors que le code actuel donne `.`.

## Modifications après le rendu

Pour cette version, j'ai retiré les commentaires et messages personnels, ajouté une vérification de lecture et de taille, et sorti du code le texte chiffré du sujet, désormais passé en argument.
