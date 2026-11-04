#define __USE_MINGW_ANSI_STDIO 1
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define WAV_HEADER_SIZE 44

// Structure pour les données audio
typedef struct {
    uint8_t *data;
    size_t size;
} WavData;

//  Fonction ajoutée : lire 2 octets 
int lire_unite16(const uint8_t *src) {
    int bas  = src[0];
    int haut = src[1];
    return bas + haut * 256;
}

// Fonction SIMPLE : Vérifier si c'est un WAV (lettre par lettre)
bool verifier_signature_wav(uint8_t *buffer) {

    if (buffer[0] != 'R' || buffer[1] != 'I' ||
        buffer[2] != 'F' || buffer[3] != 'F')
        return false;

    if (buffer[8] != 'W' || buffer[9] != 'A' ||
        buffer[10] != 'V' || buffer[11] != 'E')
        return false;

    return true;
}

// Fonction SIMPLE : Copier des octets un par un 
void copier_octets(uint8_t *destination, uint8_t *source, size_t nombre) {
    for (size_t i = 0; i < nombre; i++) {
        destination[i] = source[i];
    }
}

// Fonction : Charge un fichier WAV en mémoire
bool charger_fichier_wav(const char *filename, WavData *out_wav) {

    FILE *file = fopen(filename, "rb");
    out_wav->data = NULL;
    out_wav->size = 0;

    if (!file) {
        printf("Erreur : impossible d'ouvrir %s\n", filename);
        return false;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *buffer = malloc(file_size);
    if (!buffer) {
        printf("Erreur : allocation mémoire échouée\n");
        fclose(file);
        return false;
    }

    size_t lus = fread(buffer, 1, file_size, file);
    fclose(file);

    if (lus != file_size || file_size < WAV_HEADER_SIZE) {
        printf("Erreur : %s est illisible ou trop court\n", filename);
        free(buffer);
        return false;
    }

    if (!verifier_signature_wav(buffer)) {
        printf("Erreur : %s n'est pas un fichier WAV valide\n", filename);
        free(buffer);
        return false;
    }

    out_wav->size = file_size - WAV_HEADER_SIZE;
    out_wav->data = malloc(out_wav->size);
    
    if (!out_wav->data) {
        printf("Erreur : allocation données audio échouée\n");
        free(buffer);
        return false;
    }

    copier_octets(out_wav->data, buffer + WAV_HEADER_SIZE, out_wav->size);
    free(buffer);

    return true;
}

// MÉTHODE 1 : lecture LSB directe
void essayer_lecture_lsb_directe(WavData modified) {
    printf("\n--- Lecture LSB directe ---\n");

    size_t nb_bouts_audio = modified.size / 2;
    uint8_t bits_actifs = 0;
    int bit_contab = 0;

    printf("Resultat : ");

    for (size_t i = 0; i < nb_bouts_audio && i < 2000; i++) {

        int bout_audio = lire_unite16(modified.data + i*2);  
        uint8_t lsb = bout_audio & 0x01;

        bits_actifs = (bits_actifs << 1) | lsb;
        bit_contab++;

        if (bit_contab == 8) {
            if (bits_actifs >= 32 && bits_actifs <= 126) printf("%c", bits_actifs);
            else printf(".");
            bits_actifs = 0;
            bit_contab = 0;
        }
    }
    printf("\n");
}


// MÉTHODE 3 : XOR
void reveler_message_par_xor(WavData original, WavData modified) {

    printf("\n=== Methode XOR ===\n");

    size_t min_size = (original.size < modified.size) ? original.size : modified.size;
    size_t nb_bouts_audio = min_size / 2;

    uint8_t bits_actifs = 0;
    int bit_contab = 0;

    printf("Message extrait :\n> ");

    for (size_t i = 0; i < nb_bouts_audio; i++) {

        int bout_audio_orig = lire_unite16(original.data + i*2);   
        int bout_audio_mod  = lire_unite16(modified.data + i*2);   

        int diff = bout_audio_orig ^ bout_audio_mod;
        uint8_t bit = diff & 0x01;

        bits_actifs = (bits_actifs << 1) | bit;
        bit_contab++;

        if (bit_contab == 8) {

            if (bits_actifs >= 0x20 && bits_actifs <= 0x7E)
                printf("%c", bits_actifs);

            else if (bits_actifs == 0x00)
                break;

            else
                printf(".");

            bits_actifs = 0;
            bit_contab = 0;
        }
    }

    printf("\n\n=== Fin de l'extraction ===\n");
}


// Statistiques 

void afficher_stats(WavData original, WavData modified) {

    printf("\n--- Analyse statistique ---\n");

    size_t min_size = (original.size < modified.size) ? original.size : modified.size;
    size_t nb_bouts_audio = min_size / 2;

    int contabilise_difference = 0;
    size_t sommet = (nb_bouts_audio < 1000) ? nb_bouts_audio : 1000;

    for (size_t i = 0; i < sommet; i++) {

        int bout_audio1 = lire_unite16(original.data + i*2);  
        int bout_audio2 = lire_unite16(modified.data + i*2);

        if (bout_audio1 != bout_audio2)
            contabilise_difference++;
    }

    printf("Sur les %zu premiers echantillons, %d sont differents.\n",
        sommet, contabilise_difference);
}

// Phase 3 : Décryptage César
void decoder_cesar(const char *texte_brut, char *resultat) {

    size_t len = strlen(texte_brut);
    size_t j = 0;

    for (size_t i = 0; i < len; i++) {

        char c = texte_brut[i];

        // Gestion du "333" => "..."
        if (c=='3' && texte_brut[i+1]=='3' && texte_brut[i+2]=='3') {
            resultat[j++] = '.';
            resultat[j++] = '.';
            resultat[j++] = '.';
            i += 2;
            continue;
        }

        if (c == '%') { resultat[j++] = ' '; continue; }
        if (c == '{') { resultat[j++] = 'v'; continue; }
        if (c == '&') { resultat[j++] = '.'; continue; }

        if (c >= 'a' && c <= 'z')
            resultat[j++] = 'a' + ((c - 'a' - 5 + 26) % 26);

        else if (c >= 'A' && c <= 'Z')
            resultat[j++] = 'A' + ((c - 'A' - 5 + 26) % 26);

        else if (c >= '0' && c <= '9')
            resultat[j++] = '0' + ((c - '0' - 5 + 10) % 10);

        else
            resultat[j++] = c;
    }

    resultat[j] = '\0';
}


// MAIN
int main(int argc, char *argv[]) {

    if (argc != 3 && argc != 4) {
        printf("Usage : %s <original.wav> <modifie.wav> [texte_cesar]\n", argv[0]);
        return 1;
    }

    WavData original, modified;

    printf("Chargement...\n");

    bool succes_orig = charger_fichier_wav(argv[1], &original);
    bool succes_mod  = charger_fichier_wav(argv[2], &modified);

    if (!succes_orig || !succes_mod) {
        if (succes_orig) free(original.data);
        if (succes_mod) free(modified.data);
        return 1;
    }

    afficher_stats(original, modified);
    reveler_message_par_xor(original, modified);

    free(original.data);
    free(modified.data);

    // Le texte chiffre est passe en argument : celui du sujet reste hors du depot
    if (argc == 4) {
        char *message_decode = malloc(strlen(argv[3]) + 1);
        if (!message_decode) return 1;
        decoder_cesar(argv[3], message_decode);
        printf("\n==== Message decode (Cesar) ====\n%s\n", message_decode);
        free(message_decode);
    }

    return 0;
}

