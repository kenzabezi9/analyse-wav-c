#!/usr/bin/env python3
"""Genere des WAV synthetiques (aucun extrait de film) pour tester les deux outils."""
import struct
import sys

SECRET_FIN = b"Message cache en fin de fichier"
SECRET_LSB = b"Secret LSB"


def entete(taille_audio):
    return (b"RIFF" + struct.pack("<I", 36 + taille_audio) + b"WAVEfmt "
            + struct.pack("<IHHIIHH", 16, 1, 1, 11025, 11025, 1, 8)
            + b"data" + struct.pack("<I", taille_audio))


dossier = sys.argv[1]
audio = bytearray((i * 7) % 256 for i in range(4000))
open(dossier + "/original.wav", "wb").write(entete(len(audio)) + audio)
open(dossier + "/avec_fin.wav", "wb").write(entete(len(audio)) + audio + SECRET_FIN)

# Un bit par "echantillon" de 2 octets, comme le lit comparer_wav_xor
modifie = bytearray(audio)
bits = [(c >> (7 - k)) & 1 for c in SECRET_LSB for k in range(8)]
for i, bit in enumerate(bits):
    modifie[i * 2] ^= bit
open(dossier + "/modifie.wav", "wb").write(entete(len(modifie)) + modifie)
