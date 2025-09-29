#!/bin/bash
# 🌱 Forenzo backup + compile script with Euler-prime breath logging

# Paths
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

# ---------------------------------------------------
# Euler-prime harmonic: log breath cycle
# ---------------------------------------------------
# Approximate Euler constant
E=2.718
# Read previous cycle number
LOG_FILE="$SAN_DISK/breath_cycle.log"
if [ ! -f "$LOG_FILE" ]; then
  CYCLE=1
else
  CYCLE=$(($(tail -n 1 "$LOG_FILE" | awk '{print $1}') + 1))
fi

# Function: nearest prime
nearest_prime() {
  n=$1
  while :; do
    is_prime=1
    for ((i=2; i*i<=n; i++)); do
      if (( n % i == 0 )); then
        is_prime=0
        break
      fi
    done
    if (( is_prime == 1 )); then
      echo $n
      return
    fi
    ((n++))
  done
}

# Calculate harmonic step
ROUNDED=$(printf "%.0f" "$(echo "$E * $CYCLE" | bc -l)")
PRIME=$(nearest_prime $ROUNDED)

# Log with date
echo "$CYCLE $PRIME $DATE" >> "$LOG_FILE"
echo "🌀 Breath cycle logged: cycle=$CYCLE, prime=$PRIME"