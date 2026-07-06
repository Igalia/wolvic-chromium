#!/usr/bin/env bash
#
# Symbolize an Android native crash (tombstone / logcat DEBUG dump) against the
# locally-built unstripped libcontent_native.so.
#
# Usage:
#   wolvic/tools/symbolize-crash.sh [crash-report-file]
#   adb logcat -d | wolvic/tools/symbolize-crash.sh
#   xclip -o    | wolvic/tools/symbolize-crash.sh      # paste from clipboard
#
# Reads the crash text from the file argument or stdin, pulls out the
# `#NN pc <offset> .../libcontent_native.so` frames, checks the tombstone
# BuildId against the on-disk unstripped lib (ABORTS on mismatch — mismatched
# offsets symbolize to garbage), then prints each frame symbolized.
#
# Overridable via env:
#   LIB=<path to lib.unstripped/libcontent_native.so>   (default: out/android)
#   SYMBOLIZER=<path to llvm-symbolizer>
#   READELF=<path to llvm-readelf>
#
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

LIB="${LIB:-$SRC_ROOT/out/android/lib.unstripped/libcontent_native.so}"
LLVM_BIN="$SRC_ROOT/third_party/llvm-build/Release+Asserts/bin"
SYMBOLIZER="${SYMBOLIZER:-$LLVM_BIN/llvm-symbolizer}"
READELF="${READELF:-$LLVM_BIN/llvm-readelf}"
LIB_NAME="$(basename "$LIB")"

red()  { printf '\033[31m%s\033[0m\n' "$*" >&2; }
cyan() { printf '\033[36m%s\033[0m\n' "$*" >&2; }

[[ -x "$SYMBOLIZER" ]] || { red "llvm-symbolizer not found: $SYMBOLIZER"; exit 1; }
[[ -f "$LIB" ]]        || { red "unstripped lib not found: $LIB  (build it, or set LIB=)"; exit 1; }

report="$(cat "${1:-/dev/stdin}")"

# --- BuildId gate ----------------------------------------------------------
lib_id="$("$READELF" -n "$LIB" 2>/dev/null | grep -ioE 'Build ID: *[0-9a-f]+' | grep -oiE '[0-9a-f]{8,}' | head -1)"
crash_id="$(printf '%s\n' "$report" | grep -ioE 'BuildId: *[0-9a-f]+' | grep -oiE '[0-9a-f]{8,}' | head -1)"
if [[ -n "$crash_id" ]]; then
  if [[ "${crash_id,,}" == "${lib_id,,}" ]]; then
    cyan "BuildId OK ($lib_id) — $LIB"
  else
    red "BuildId MISMATCH — the on-disk lib is NOT this crash's build; offsets would be garbage."
    red "  crash: $crash_id"
    red "  lib:   $lib_id"
    red "Rebuild for this crash or set LIB= to the matching unstripped lib. Aborting."
    exit 2
  fi
else
  cyan "No BuildId found in the report — cannot verify. Results are only valid if $LIB_NAME matches the crashing build."
fi

# --- Extract frames for the target lib and symbolize -----------------------
# Frame lines look like (with optional logcat prefix):
#   ... #00 pc 00000000098041d4  /data/.../libcontent_native.so (BuildId: ...)
mapfile -t frames < <(
  printf '%s\n' "$report" \
    | grep -E "#[0-9]+[[:space:]]+pc[[:space:]]+[0-9a-fA-F]+" \
    | grep -F "$LIB_NAME"
)

if [[ ${#frames[@]} -eq 0 ]]; then
  red "No '$LIB_NAME' frames found in the report."
  exit 3
fi

cyan "Symbolizing ${#frames[@]} frame(s):"
for line in "${frames[@]}"; do
  num="$(grep -oE '#[0-9]+' <<<"$line" | head -1)"
  off="$(grep -oiE 'pc[[:space:]]+[0-9a-fA-F]+' <<<"$line" | grep -oiE '[0-9a-f]+$' | head -1)"
  [[ -n "$off" ]] || continue
  # -f function name, -C demangle. No -i: one function per frame.
  out="$("$SYMBOLIZER" -e "$LIB" -f -C "0x$off" 2>/dev/null)"
  # Method name only: protect "(anonymous namespace)", then drop the param list.
  func="$(sed -n '1p' <<<"$out" | sed 's/(anonymous namespace)/[anon]/g; s/(.*//')"
  [[ -n "$func" ]] || func="??"
  # Location -> basename:line (drop the path and the :column).
  loc="$(sed -n '2p' <<<"$out")"
  loc="$(basename -- "${loc:-?}" 2>/dev/null | sed 's/:[0-9]*$//')"
  printf '%-4s %s  (%s)\n' "$num" "$func" "$loc"
done
