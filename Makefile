CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c11 -g -fsanitize=address,undefined
BUILD := build

all: $(BUILD)/detecter_fin_wav $(BUILD)/comparer_wav_xor

$(BUILD)/%: src/%.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

test: all
	python3 tests/generer_wav.py $(BUILD)
	$(BUILD)/detecter_fin_wav $(BUILD)/avec_fin.wav | grep -q "Message cache en fin de fichier"
	$(BUILD)/detecter_fin_wav $(BUILD)/original.wav | grep -q "Aucune donnee cachee"
	$(BUILD)/comparer_wav_xor $(BUILD)/original.wav $(BUILD)/modifie.wav | grep -q "Secret LSB"
	$(BUILD)/comparer_wav_xor $(BUILD)/original.wav $(BUILD)/modifie.wav "Gtsotzw" | grep -q "Bonjour"
	@echo "Tous les tests passent."

clean:
	rm -rf $(BUILD)

.PHONY: all test clean
