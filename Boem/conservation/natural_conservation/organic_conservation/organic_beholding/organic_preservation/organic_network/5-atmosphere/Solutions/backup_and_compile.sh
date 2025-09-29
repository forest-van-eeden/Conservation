#!/bin/bash
# 🌱 Forenzo backup + compile script
# Supports Organic Preservation cycles

# Paths (edit these if your setup differs)
SOURCE="forenzo.c"
SAN_DISK="/Volumes/©eden/Forenzo/versions"
DATE=$(date +"%Y-%m-%d")
VERSION="v0.5"

# Ensure SanDisk path exists
mkdir -p "$SAN_DISK"

# Backup with version + date
BACKUP_FILE="$SAN_DISK/forenzo_${VERSION}_${DATE}.c"
cp "$SOURCE" "$BACKUP_FILE"

echo "📦 Backed up $SOURCE → $BACKUP_FILE"

# Compile
clang -o forenzo "$SOURCE"
if [ $? -eq 0 ]; then
  echo "✅ Compilation successful: ./forenzo ready"
else
  echo "❌ Compilation failed."
  exit 1
fi
