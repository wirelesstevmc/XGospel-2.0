# XGospel2 Qt5 Makefile
# Comprehensive build system for Qt5-based Go client

# Compiler and basic flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -g -fPIC
DEFINES = -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_CORE_LIB

# Qt5 configuration
QT5_CFLAGS = $(shell pkg-config --cflags Qt5Widgets Qt5Network Qt5Core Qt5Gui)
QT5_LIBS = $(shell pkg-config --libs Qt5Widgets Qt5Network Qt5Core Qt5Gui)

# Include directories  
INCLUDES = -I. $(QT5_CFLAGS)

# All compiler flags combined
ALL_CXXFLAGS = $(CXXFLAGS) $(DEFINES) $(INCLUDES)

# Source files
MAIN_SOURCE = xgospel2_fixed.cpp
SOURCES = board_window.cpp \
          igs_move_parser.cpp \
          q5go_parser.cpp \
          q5go_games_parser.cpp \
          game_tree.cpp \
          settings.cpp \
          preferences_dialog.cpp

# Object files
MAIN_OBJECT = $(MAIN_SOURCE:.cpp=.o)
OBJECTS = $(SOURCES:.cpp=.o)

# MOC files (Qt meta-object compiler)
MOC_HEADERS = board_window.h preferences_dialog.h
MOC_SOURCES = $(MOC_HEADERS:.h=.moc) xgospel2_fixed.moc

# Target executables
TARGET = xgospel2
SGF_TARGET = xgospel2_sgf_fix

# Default target
all: $(TARGET)

# Main executable
$(TARGET): $(MAIN_OBJECT) $(OBJECTS) $(MOC_SOURCES)
	@echo "Linking $(TARGET)..."
	$(CXX) -o $@ $(MAIN_OBJECT) $(OBJECTS) $(QT5_LIBS)
	@echo "Build complete: $(TARGET)"

# SGF fix version 
$(SGF_TARGET): $(MAIN_OBJECT) $(OBJECTS) $(MOC_SOURCES)
	@echo "Linking $(SGF_TARGET)..."
	$(CXX) -o $@ $(MAIN_OBJECT) $(OBJECTS) $(QT5_LIBS)
	@echo "SGF fix build complete: $(SGF_TARGET)"

# Compile source files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(ALL_CXXFLAGS) -c $< -o $@

# Qt5 MOC path
QT5_MOC = /usr/lib64/qt5/bin/moc

# Generate MOC files from headers
%.moc: %.h
	@echo "MOC processing header $<..."
	$(QT5_MOC) $< -o $@

# Generate MOC file from main cpp (contains Q_OBJECT classes)
xgospel2_fixed.moc: xgospel2_fixed.cpp
	@echo "MOC processing main cpp..."
	$(QT5_MOC) xgospel2_fixed.cpp -o xgospel2_fixed.moc

# Build both versions
both: $(TARGET) $(SGF_TARGET)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f *.o *.moc $(TARGET) $(SGF_TARGET)
	@echo "Clean complete."

# Rebuild everything
rebuild: clean all

# Debug build
debug: CXXFLAGS += -DDEBUG -g3 -O0
debug: clean all

# Show build configuration
config:
	@echo "Build Configuration:"
	@echo "CXX: $(CXX)"
	@echo "CXXFLAGS: $(CXXFLAGS)" 
	@echo "QT5_CFLAGS: $(QT5_CFLAGS)"
	@echo "QT5_LIBS: $(QT5_LIBS)"
	@echo "Main source: $(MAIN_SOURCE)"
	@echo "Sources: $(SOURCES)"
	@echo "Target: $(TARGET)"

# Install
install: $(TARGET)
	@echo "Installing $(TARGET)..."
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Installation complete."

# Help
help:
	@echo "Available targets:"
	@echo "  all          - Build main executable (default)"
	@echo "  xgospel2_sgf_fix - Build SGF debugging version"  
	@echo "  both         - Build both versions"
	@echo "  clean        - Remove all build artifacts"
	@echo "  rebuild      - Clean and build"
	@echo "  debug        - Build with debug symbols"
	@echo "  install      - Install to /usr/local/bin"
	@echo "  config       - Show build configuration"
	@echo "  help         - Show this help"

# Declare phony targets
.PHONY: all clean rebuild debug install config help both

# Dependencies
$(MAIN_OBJECT): $(MAIN_SOURCE) xgospel2_fixed.moc q5go_parser.h q5go_games_parser.h igs_move_parser.h settings.h
board_window.o: board_window.cpp board_window.h board_window.moc igs_move_parser.h
igs_move_parser.o: igs_move_parser.cpp igs_move_parser.h
q5go_parser.o: q5go_parser.cpp q5go_parser.h
q5go_games_parser.o: q5go_games_parser.cpp q5go_games_parser.h
settings.o: settings.cpp settings.h
preferences_dialog.o: preferences_dialog.cpp preferences_dialog.h preferences_dialog.moc settings.h
board_window.moc: board_window.h
preferences_dialog.moc: preferences_dialog.h
xgospel2_fixed.moc: xgospel2_fixed.cpp