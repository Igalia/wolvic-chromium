#!/usr/bin/env bash
# migrate-apis.sh — turn the drift report into proposed code changes (semi-automatic migration).
#
# Reads drift-report-m<build>.json (produced by api-drift-report.sh) and, for each breaking/review
# entry, asks a headless `claude -p` to propose how to re-apply the Wolvic intent on the new
# upstream API. Writes one suggestion per file under ./suggestions/ as a unified diff for human
# review. It does NOT auto-apply or commit (unless you pass --apply, which only writes to the
# working tree, still unstaged).
#
# Usage:
#   migrate-apis.sh                      # all breaking+review entries in the newest report
#   migrate-apis.sh --file <path>        # just one file (use during a rebase conflict)
#   migrate-apis.sh --all                # include 'ok' entries too
#   migrate-apis.sh --json <report.json> # explicit report
#   migrate-apis.sh --apply              # also git-apply the suggestion (unstaged) for review
#
# Env: CLAUDE_BIN (default: claude), CLAUDE_MODEL (optional).

set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "$HERE/lib.sh"

CLAUDE_BIN="${CLAUDE_BIN:-claude}"
JSON=""; ONLY_FILE=""; INCLUDE_OK=0; APPLY=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --json)  JSON="$2"; shift 2 ;;
    --file)  ONLY_FILE="$2"; shift 2 ;;
    --all)   INCLUDE_OK=1; shift ;;
    --apply) APPLY=1; shift ;;
    -h|--help) sed -n '2,22p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) die "unknown arg '$1'" ;;
  esac
done

command -v "$CLAUDE_BIN" >/dev/null || die "'$CLAUDE_BIN' not found on PATH (set CLAUDE_BIN)"

if [[ -z "$JSON" ]]; then
  JSON="$(ls -t "$HERE"/drift-report-m*.json 2>/dev/null | head -1 || true)"
  [[ -n "$JSON" ]] || die "no drift-report-m*.json found; run api-drift-report.sh first"
fi
[[ -f "$JSON" ]] || die "report not found: $JSON"
log "using report: $JSON"

SUGG_DIR="$HERE/suggestions"
mkdir -p "$SUGG_DIR"
ROOT="$(repo_root)"

# Select entries: by default breaking+review; --all adds ok; --file narrows to one path.
mapfile -t SELECTED < <(
  python3 - "$JSON" "$ONLY_FILE" "$INCLUDE_OK" <<'PY'
import json, sys
data = json.load(open(sys.argv[1])); only, inc_ok = sys.argv[2], sys.argv[3] == "1"
for i, e in enumerate(data["entries"]):
    if only and e["wolvic_file"] != only: continue
    if e["severity"] == "ok" and not inc_ok: continue
    print(i)
PY
)
[[ ${#SELECTED[@]} -gt 0 ]] || { log "nothing to migrate (no matching entries)."; exit 0; }
log "${#SELECTED[@]} entr(y/ies) to process."

emit_prompt() {  # $1 = entry index -> prints the prompt for claude on stdout
  python3 - "$JSON" "$1" "$ROOT" <<'PY'
import json, os, sys
data = json.load(open(sys.argv[1])); e = data["entries"][int(sys.argv[2])]; root = sys.argv[3]
path = e["wolvic_file"]
cur = ""
fp = os.path.join(root, path)
if os.path.isfile(fp):
    cur = open(fp, encoding="utf-8", errors="ignore").read()
print(f"""You are migrating a downstream Chromium fork (Wolvic) across a milestone jump
(old_base {data['old_base'][:12]} -> new_base {data['new_base'][:12]}, m{data['milestone_build']}).

FILE: {path}
GROUP: {e['group']}   KIND: {e['kind']}   UPSTREAM_STATUS: {e['upstream_status']}   SEVERITY: {e['severity']}
RENAMED_TO: {e.get('renamed_to')}
WOLVIC SYMBOLS OF INTEREST: {', '.join(e.get('symbols') or []) or '(n/a)'}

WHAT UPSTREAM CHANGED between old and new (base..new):
```diff
{e.get('upstream_diff') or '(file deleted upstream, or header — see status)'}
```

THE WOLVIC PATCH we must preserve the INTENT of (base..wolvic):
```diff
{e.get('wolvic_patch') or '(no in-tree patch; this is a header dependency our code includes)'}
```

CURRENT FILE CONTENT at new_base (what we are editing now):
```
{cur[:18000] if cur else '(file not present in working tree — likely deleted/renamed upstream)'}
```

TASK: Re-apply the Wolvic change on top of the NEW upstream API. Output ONLY a unified git diff
(diff --git a/{path} b/{path} ...) that can be applied with `git apply`. If the upstream file was
deleted/renamed, instead output a short plan (prefixed `PLAN:`) describing where the Wolvic hook
should move to and what the new symbol is. If our patch still applies cleanly, say `CLEAN:` and
nothing else. Do not explain; output the diff/PLAN/CLEAN only.""")
PY
}

run_one() {
  local idx="$1" path slug out
  path="$(python3 -c 'import json,sys;print(json.load(open(sys.argv[1]))["entries"][int(sys.argv[2])]["wolvic_file"])' "$JSON" "$idx")"
  slug="$(echo "$path" | tr '/.' '__')"
  out="$SUGG_DIR/${slug}.patch"
  log "  -> $path"
  local args=(-p --output-format text)
  [[ -n "${CLAUDE_MODEL:-}" ]] && args+=(--model "$CLAUDE_MODEL")
  if emit_prompt "$idx" | "$CLAUDE_BIN" "${args[@]}" > "$out" 2>/dev/null; then
    if head -1 "$out" | grep -q '^CLEAN:'; then
      log "     clean — our patch still applies; removing empty suggestion"; rm -f "$out"
    elif head -1 "$out" | grep -q '^PLAN:'; then
      log "     PLAN (manual): $out"
    else
      log "     suggestion: $out"
      if (( APPLY )); then
        if git -C "$ROOT" apply --3way "$out" 2>/dev/null; then
          log "     applied to working tree (unstaged) — review before 'git add'"
        else
          warn "     could not auto-apply; apply $out by hand"
        fi
      fi
    fi
  else
    warn "     claude invocation failed for $path"
  fi
}

for idx in "${SELECTED[@]}"; do run_one "$idx"; done
log "done. Review suggestions in $SUGG_DIR/ — nothing was committed."
