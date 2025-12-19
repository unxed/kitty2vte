CC = gcc
CXX = g++
RUSTC = rustc

BUILD_DIR = build
EXEC_DIR = $(BUILD_DIR)/bin

KITTY_CFLAGS = -Wall -Wextra -std=c11

VTE_CXXFLAGS = -Wall -Wextra -std=c++17 -DVTE_GTK=4 `pkg-config --cflags xkbcommon`
VTE_LDFLAGS = `pkg-config --libs xkbcommon`

FAR2L_CXXFLAGS = -Wall -Wextra -std=c++17

KITTY_TESTER = $(EXEC_DIR)/kitty_tester
VTE_TESTER = $(EXEC_DIR)/vte_tester
FAR2L_TESTER = $(EXEC_DIR)/far2l_tester
ALACRITTY_TESTER = $(EXEC_DIR)/alacritty_tester

.PHONY: all clean

all: $(BUILD_DIR) $(EXEC_DIR) $(KITTY_TESTER) $(VTE_TESTER) $(FAR2L_TESTER) $(ALACRITTY_TESTER)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/kitty
	mkdir -p $(BUILD_DIR)/vte
	mkdir -p $(BUILD_DIR)/far2l
	mkdir -p $(BUILD_DIR)/alacritty

$(EXEC_DIR):
	mkdir -p $(EXEC_DIR)

# Kitty Rules

kitty_test/kitty_encoder_body.inc: source/key_encoding.c kitty_test/extract_kitty.py
	@echo "=> Generating kitty encoder body..."
	@python3 kitty_test/extract_kitty.py source/key_encoding.c

$(BUILD_DIR)/kitty/kitty_tester.o: kitty_test/kitty_tester.c kitty_test/kitty_mocks.h kitty_test/kitty_encoder_body.inc
	@echo "=> Compiling kitty tester object..."
	$(CC) $(KITTY_CFLAGS) -c kitty_test/kitty_tester.c -o $@

$(KITTY_TESTER): $(BUILD_DIR)/kitty/kitty_tester.o
	@echo "=> Linking kitty tester..."
	$(CC) $^ -o $@
	@echo "-> Built $(KITTY_TESTER)"

# VTE Rules

vte_test/vte_key_press_body.inc: source/vte.cc vte_test/extract_code.py
	@echo "=> Generating VTE key press body..."
	@python3 vte_test/extract_code.py source/vte.cc

$(BUILD_DIR)/vte/main.o: vte_test/main.cc vte_test/vte_key_tester.h
	@echo "=> Compiling VTE tester main object..."
	$(CXX) $(VTE_CXXFLAGS) -c vte_test/main.cc -o $@

$(BUILD_DIR)/vte/vte_key_tester.o: vte_test/vte_key_tester.cc vte_test/vte_key_tester.h vte_test/kittykeys.h vte_test/vte_key_press_body.inc
	@echo "=> Compiling VTE tester logic object..."
	$(CXX) $(VTE_CXXFLAGS) -c vte_test/vte_key_tester.cc -o $@

$(VTE_TESTER): $(BUILD_DIR)/vte/main.o $(BUILD_DIR)/vte/vte_key_tester.o
	@echo "=> Linking VTE tester..."
	$(CXX) $^ -o $@ $(VTE_LDFLAGS)
	@echo "-> Built $(VTE_TESTER)"

# Far2l Rules

far2l_test/far2l_key_press_body.inc: source/vtshell_translation_kitty.cpp far2l_test/extract_far2l.py
	@echo "=> Generating Far2l key press body..."
	@python3 far2l_test/extract_far2l.py source/vtshell_translation_kitty.cpp

$(BUILD_DIR)/far2l/far2l_tester.o: far2l_test/far2l_tester.cpp far2l_test/far2l_mocks.h far2l_test/far2l_key_press_body.inc
	@echo "=> Compiling Far2l tester object..."
	$(CXX) $(FAR2L_CXXFLAGS) -c far2l_test/far2l_tester.cpp -o $@

$(FAR2L_TESTER): $(BUILD_DIR)/far2l/far2l_tester.o
	@echo "=> Linking Far2l tester..."
	$(CXX) $^ -o $@
	@echo "-> Built $(FAR2L_TESTER)"

# Alacritty Rules

alacritty_test/alacritty_extracted.rs: source/keyboard.rs alacritty_test/extract_alacritty.py
	@echo "=> Generating Alacritty extracted logic..."
	@python3 alacritty_test/extract_alacritty.py source/keyboard.rs

$(ALACRITTY_TESTER): alacritty_test/alacritty_tester.rs alacritty_test/alacritty_mocks.rs alacritty_test/alacritty_extracted.rs
	@echo "=> Compiling Alacritty tester..."
	$(RUSTC) alacritty_test/alacritty_tester.rs -o $@
	@echo "-> Built $(ALACRITTY_TESTER)"

clean:
	@echo "=> Cleaning build files..."
	rm -rf $(BUILD_DIR)
	rm -f vte_test/vte_key_press_body.inc kitty_test/kitty_encoder_body.inc far2l_test/far2l_key_press_body.inc alacritty_test/alacritty_extracted.rs
