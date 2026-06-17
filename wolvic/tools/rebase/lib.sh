#!/usr/bin/env bash
# lib.sh — shared helpers for the Wolvic rebase tooling.
#
# Sourced by rebase-wolvic.sh, api-drift-report.sh and migrate-apis.sh. Holds the logic that
# locates the fork point and resolves Chromium milestones to branch-heads, so the three tools
# agree on OLD_BASE / NEW_BASE without duplicating it.

set -euo pipefail

WOLVIC_BRANCH="${WOLVIC_BRANCH:-wolvic}"
CHROMIUM_SRC_URL="${CHROMIUM_SRC_URL:-https://chromium.googlesource.com/chromium/src}"
CHROMIUMDASH="${CHROMIUMDASH:-https://chromiumdash.appspot.com}"

# Local tracking refs the `fetch` step writes; every other step reads them so we never depend on
# the volatile FETCH_HEAD across separate invocations.
REF_OLD="refs/wolvic-rebase/old-base"   # tip of the OLD milestone's branch-heads
REF_NEW="refs/wolvic-rebase/new-base"   # tip of the NEW milestone's branch-heads

log()  { printf '\033[1;34m[rebase]\033[0m %s\n' "$*" >&2; }
warn() { printf '\033[1;33m[warn]\033[0m %s\n'   "$*" >&2; }
die()  { printf '\033[1;31m[error]\033[0m %s\n'  "$*" >&2; exit 1; }

repo_root() { git -C "${1:-.}" rev-parse --show-toplevel; }

# The third component of chrome/VERSION. For modern Chromium this *is* the branch-heads number
# (M124 -> 6367, M150 -> 7871).
current_build() {
  local root; root="$(repo_root)"
  awk -F= '/^BUILD=/{print $2}' "$root/chrome/VERSION"
}

# Map a CLI argument to a branch-heads build number.
#   - 4+ digits (e.g. 7871) -> taken as the branch-heads number directly.
#   - <=3 digits (e.g. 150) -> treated as a milestone and resolved via chromiumdash.
resolve_build() {
  local arg="$1"
  if [[ "$arg" =~ ^[0-9]{4,}$ ]]; then
    echo "$arg"; return
  fi
  [[ "$arg" =~ ^[0-9]{1,3}$ ]] || die "target '$arg' is neither a milestone nor a branch number"
  local build
  # chromiumdash ignores the mstone filter, so one wide query returns every milestone's releases;
  # we match the target milestone in Python. A milestone is tagged on many builds (pre-branch
  # Canary/Dev have lower builds than the branch point); the branch-heads number is the *max*
  # build — the value the branch was cut at and that stable/extended stay on.
  build="$(curl -fsSL "${CHROMIUMDASH}/fetch_releases?channel=Stable,Extended,Beta,Dev,Canary&num=50" 2>/dev/null \
    | python3 -c '
import sys, json
target = int(sys.argv[1])
try:
    d = json.load(sys.stdin)
except Exception:
    sys.exit(0)
found = []
def walk(o):
    if isinstance(o, dict):
        ms = o.get("milestone", o.get("mstone"))
        v = o.get("version")
        if ms == target and isinstance(v, str) and v.count(".") == 3:
            found.append(int(v.split(".")[2]))
        for x in o.values():
            walk(x)
    elif isinstance(o, list):
        for x in o:
            walk(x)
walk(d)
print(max(found) if found else "")
' "$arg" 2>/dev/null)"
  [[ -n "$build" ]] && { echo "$build"; return; }
  die "could not resolve milestone $arg to a branch-heads number via chromiumdash; pass the build number explicitly"
}

# Verify a branch-heads ref exists upstream and echo its sha (does not fetch objects).
verify_branch_head() {
  local build="$1" sha
  sha="$(git ls-remote "$CHROMIUM_SRC_URL" "refs/branch-heads/${build}" 2>/dev/null | awk '{print $1}')"
  [[ -n "$sha" ]] || die "refs/branch-heads/${build} does not resolve on $CHROMIUM_SRC_URL"
  echo "$sha"
}

# OLD_BASE = the upstream commit the wolvic branch forked from = merge-base of the old milestone's
# branch-heads tip and wolvic. Requires `fetch` to have populated REF_OLD.
old_base() {
  git rev-parse --verify --quiet "$REF_OLD" >/dev/null \
    || die "$REF_OLD missing — run 'rebase-wolvic.sh fetch' first"
  git merge-base "$REF_OLD" "$WOLVIC_BRANCH"
}

new_base() {
  git rev-parse --verify --quiet "$REF_NEW" >/dev/null \
    || die "$REF_NEW missing — run 'rebase-wolvic.sh fetch' first"
  git rev-parse "$REF_NEW"
}
