# ------------------------------------------------------------------------------
# CONFIGURAZIONE
# Strumenti, sorgenti e opzioni. Va per prima: le variabili usate
# nell'intestazione delle regole devono essere già definite quando
# make le legge.
# ------------------------------------------------------------------------------

# Il compilatore incrociato ARM: gira sul PC, produce codice per Cortex-M (notes §2)
CC := arm-none-eabi-gcc

# La cartella dove finiscono tutti i file generati
BUILD_DIR := build

# L'immagine finale
BIN := $(BUILD_DIR)/scratch-os.axf

# I sorgenti: il codice di avvio e l'applicazione
SOURCES := boot/startup.c src/main.c

# I file oggetto, ricavati da SOURCES con un riferimento di sostituzione
# (substitution reference): src/main.c -> build/src/main.o (notes §6)
OBJECTS := $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# Opzioni del compilatore: processore e ambiente, avvisi, debug (notes §2)
CFLAGS := -mthumb -mcpu=cortex-m3 -nostartfiles -ffreestanding -std=gnu17
CFLAGS += -Wall -Wextra
CFLAGS += -O0 -g3

# Opzioni del linker: script del linker, file di mappa, niente libreria C (notes §3)
LDFLAGS := -T scripts/mps2_m3.ld
LDFLAGS += -Xlinker -Map=$(BUILD_DIR)/output.map
LDFLAGS += -nostdlib -lgcc

# ------------------------------------------------------------------------------
# INTERFACCIA
# Le azioni che si possono chiedere a make: obiettivi che NON sono file.
# Precede le regole di costruzione perché `all` deve essere il primo
# obiettivo del file (obiettivo predefinito).
# ------------------------------------------------------------------------------

# Le azioni: make esegue sempre la loro ricetta, senza cercare un file omonimo
.PHONY: all run clean size disasm

# Obiettivo predefinito: `make` senza argomenti costruisce l'immagine
all: $(BIN)

# Aggiorna l'immagine se serve, poi la avvia in QEMU (uscita: Ctrl+C)
run: $(BIN)
	qemu-system-arm -machine mps2-an385 -monitor null -semihosting \
	    --semihosting-config enable=on,target=native \
	    -kernel $(BIN) -serial stdio -nographic

# Rimuove tutti i file generati
clean:
	rm -rf $(BUILD_DIR)

# Diagnostica: `make print-NOME` stampa il valore della variabile NOME (notes §6)
print-%:
	@echo "$* = $($*)"

size: $(BIN)
	arm-none-eabi-size $(BIN)

disasm: $(BIN)
	arm-none-eabi-objdump -d $(BIN)

# ------------------------------------------------------------------------------
# REGOLE DI COSTRUZIONE
# Come si producono i file: obiettivi che SONO file.
#
#   obiettivo: prerequisito1 prerequisito2 ...
#   <TAB>ricetta
# ------------------------------------------------------------------------------

## Compilazione: una regola a modello (pattern rule) per ogni file .c
#   %         = la radice, ricalcolata per ogni file (es. src/main)
#   $@        = l'obiettivo, cioè il file .o da produrre (es. build/src/main.o)
#   $<        = il primo prerequisito, cioè il sorgente (es. src/main.c)
#   $(dir $@) = la cartella dell'obiettivo (es. build/src/), da creare
#   -c        = compila senza collegare
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

## Collegamento: unisce tutti i file oggetto nell'immagine finale
#   CFLAGS serve anche qui: -mthumb e -mcpu scelgono la libgcc giusta
$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(BIN)