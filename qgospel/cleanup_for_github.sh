#!/bin/bash
# Cleanup script for GitHub release preparation
# Removes build artifacts, backups, logs, and organizes documentation

echo "=== XGospel2 GitHub Cleanup Script ==="
echo ""

# Create docs directory if it doesn't exist
if [ ! -d docs ]; then
    echo "Creating docs/ directory..."
    mkdir -p docs
fi

# Move all .md files EXCEPT CHANGELOG.md to docs/
echo "Moving documentation files to docs/..."
for file in *.md; do
    if [ "$file" != "CHANGELOG.md" ] && [ -f "$file" ]; then
        echo "  Moving $file -> docs/"
        mv "$file" docs/
    fi
done

# Create debug directory if it doesn't exist
if [ ! -d debug ]; then
    echo "Creating debug/ directory..."
    mkdir -p debug
fi

# Move useful debug binaries to debug/
echo ""
echo "Moving useful debug binaries to debug/..."
debug_binaries=(xgospel2_komi_fixed xgospel2_komi_storage_fix xgospel2_territory_marking xgospel2_auto_scoring xgospel2_server_scoring xgospel2_comprehensive_pass_fix xgospel2_final_pass_fix xgospel2_board_position_fixed)
for binary in "${debug_binaries[@]}"; do
    if [ -f "$binary" ]; then
        echo "  Moving $binary -> debug/"
        mv "$binary" debug/
    fi
done

# Remove obsolete debug binaries (executables only, not source/build files)
echo ""
echo "Removing obsolete debug binaries..."
for file in xgospel2_*; do
    # Skip if glob didn't match anything
    [ -e "$file" ] || continue

    # Skip source files, build artifacts, and backup files
    case "$file" in
        *.cpp|*.h|*.o|*.moc|*.log|*.backup*|*.bak*)
            continue
            ;;
    esac

    # Only delete if it's an executable binary
    if [ -f "$file" ] && [ -x "$file" ]; then
        echo "  Removing $file"
        rm -f "$file"
    fi
done

# Remove build artifacts
echo ""
echo "Removing build artifacts..."
echo "  Removing *.o files..."
rm -f *.o
echo "  Removing *.moc files..."
rm -f *.moc
echo "  Removing xgospel2 binary..."
rm -f xgospel2

# Remove backup files
echo ""
echo "Removing backup files..."
rm -f *.backup
rm -f *.backup_*
rm -f *.bak
rm -f *.bak1
rm -f *.bak2
rm -f board_window_komi_debug.cpp
rm -f xgospel2_komi_debug.cpp

# Remove log files
echo ""
echo "Removing log files..."
rm -f *.log

# Remove temporary patches and scripts
echo ""
echo "Removing temporary patches and scripts..."
rm -f bc_player_fix.patch
rm -f fix_bc_players.py
rm -f test_command7.cpp

# List what remains
echo ""
echo "=== Cleanup Complete ==="
echo ""
echo "Remaining source files:"
ls -1 *.cpp *.h 2>/dev/null | head -20
echo ""
echo "Documentation moved to docs/:"
doc_count=$(ls -1 docs/*.md 2>/dev/null | wc -l)
echo "  $doc_count files"
echo ""
echo "Debug binaries moved to debug/:"
debug_count=$(ls -1 debug/ 2>/dev/null | wc -l)
echo "  $debug_count binaries"
echo ""
echo "CHANGELOG.md: $(ls -lh CHANGELOG.md 2>/dev/null | awk '{print $5}')"
echo ""
echo "Ready for GitHub!"
