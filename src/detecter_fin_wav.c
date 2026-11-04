#define __USE_MINGW_ANSI_STDIO 1
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h> // Pour les bool, true, false

#define BYTES_PER_LINE 8
#define MAX_FILE_SIZE (100 * 1024 * 1024)

// Structure pour contenir tout le fichier brut
typedef struct {
    uint8_t *data;
    size_t size;
} FichierBrut;

// Fonction utilitaire pour savoir si c'est du bruit ou du texte
int est_caractere_ignorable(char c) {
    return (c == '#' || c == '@' || c == '.' || c == '\0' || (unsigned char)c == 0xFF);
}

// Fonction : Charge tout le fichier en mémoire
// Retourne : true si succès, false si erreur
bool charger_fichier_complet(const char *filename, FichierBrut *out_file) {
    FILE *file = fopen(filename, "rb");
    
    out_file->data = NULL;
    out_file->size = 0;

    if (!file) {
        printf("Erreur : impossible d'ouvrir %s\n", filename);
        return false;
    }

    // Récupérer la taille réelle du fichier
    fseek(file, 0, SEEK_END);
    out_file->size = ftell(file);
    fseek(file, 0, SEEK_SET);

    printf("Taille reelle du fichier : %lu octets\n\n", (unsigned long)out_file->size);

    if (out_file->size > MAX_FILE_SIZE) {
        printf("Erreur : fichier trop volumineux.\n");
        fclose(file);
        return false;
    }

    out_file->data = malloc(out_file->size);
    if (!out_file->data) {
        printf("Erreur : allocation memoire echouee.\n");
        fclose(file);
        return false;
    }

    size_t lus = fread(out_file->data, 1, out_file->size, file);
    fclose(file);

    if (lus != out_file->size) {
        printf("Erreur : lecture incomplete de %s\n", filename);
        free(out_file->data);
        out_file->data = NULL;
        return false;
    }

    return true; 
}

void analyser_fin_de_fichier(FichierBrut fichier) {
    uint32_t taille_declaree, subchunk2_size;
    size_t debut_cachette;
    size_t i, compteur_octets = 0;
    char hex_line[256], ascii_line[256], hex[4];

    // Un en-tete WAV standard fait au moins 44 octets
    if (fichier.size < 44) {
        printf("Erreur : fichier trop court pour etre un WAV.\n");
        return;
    }

    // Vérification signature WAV
    if (memcmp(fichier.data, "RIFF", 4) != 0 || memcmp(fichier.data + 8, "WAVE", 4) != 0) {
        printf("Erreur : ce n'est pas un WAV valide.\n");
        return;
    }

    // Lecture des infos techniques du WAV
    memcpy(&taille_declaree, fichier.data + 4, 4);
    memcpy(&subchunk2_size, fichier.data + 40, 4);

    printf("ChunkSize (ce que le fichier declare)   : %u\n", taille_declaree);
    printf("Subchunk2Size (la taille audio pure)    : %u\n", subchunk2_size);
    printf("Offset audio standard                   : 44 octets\n");

    // Calcul de l'endroit où le son est censé s'arrêter
    debut_cachette = 44 + subchunk2_size;

    printf("\nSeuil des donnees cachees (la frontiere) : %zu\n\n", debut_cachette);

    if (debut_cachette >= fichier.size) {
        printf("Aucune donnee cachee detectee.\n");
        return;
    }

    if (taille_declaree + 8 != fichier.size) {
        printf("Incoherence detectee entre la taille declaree et la taille reelle.\n");
        printf("   => Le fichier contient probablement des donnees cachees a la fin.\n\n");
    }

    printf("=== HEXA + ASCII des donnees cachees ===\n\n");

    hex_line[0] = ascii_line[0] = '\0';

    // On parcourt ce qui dépasse
    for (i = debut_cachette; i < fichier.size; i++) {

        sprintf(hex, "%02X ", fichier.data[i]);
        strcat(hex_line, hex);

        if (fichier.data[i] >= 0x20 && fichier.data[i] <= 0x7E && !est_caractere_ignorable(fichier.data[i]))
            strncat(ascii_line, (char *)&fichier.data[i], 1);
        else
            strcat(ascii_line, ".");

        compteur_octets++;

        if (compteur_octets % BYTES_PER_LINE == 0 || i == fichier.size - 1) {
            printf("%-24s | %s\n", hex_line, ascii_line);
            hex_line[0] = ascii_line[0] = '\0';
        }
    }

    printf("\n=== Extraction texte lisible  ===\n");
    for (i = debut_cachette; i < fichier.size; i++) {
        if (!est_caractere_ignorable(fichier.data[i]) && fichier.data[i] >= 0x20 && fichier.data[i] <= 0x7E)
            putchar(fichier.data[i]);
    }

    printf("\n");
}

int main(int argc, char *argv[]) {

    FichierBrut mon_fichier;
    bool succes_chargement;

    if (argc != 2) {
        printf("Usage : %s fichier.wav\n", argv[0]);
        return 1;
    }

    printf("Chargement...\n");

    succes_chargement = charger_fichier_complet(argv[1], &mon_fichier);

    if (!succes_chargement) {
        printf("Arrêt : Echec du chargement.\n");
        return 1;
    }

    // On lance l'analyse
    analyser_fin_de_fichier(mon_fichier);

    // Nettoyage
    free(mon_fichier.data);
    return 0;
}