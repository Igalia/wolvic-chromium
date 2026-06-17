#!/usr/bin/env bash
# rebase-wolvic.sh — orchestrate rebasing the Wolvic changes onto a newer Chromium milestone.
#
# Automates the mechanical parts of any milestone rebase: locating the fork point, resolving the
# target milestone to a branch-heads ref, fetching, and launching the `--onto` rebase. Conflict
# resolution itself stays manual. The slow part (the RBE build) is front-run by
# api-drift-report.sh, invoked via the `report` subcommand.
#
# Usage:
#   rebase-wolvic.sh detect-base
#   rebase-wolvic.sh resolve-target <milestone|branch-build>
#   rebase-wolvic.sh fetch          <milestone|branch-build>
#   rebase-wolvic.sh report         [milestone|branch-build]
#   rebase-wolvic.sh onto           <milestone|branch-build> [--yes]
#   rebase-wolvic.sh status
#
# Env:
#   SRC_BRANCH   branch to replay (default: wolvic). Set this if you curated the series onto a
#                separate branch first (see "Large jumps" in wolvic/REBASE.md).
#
# Nothing here pushes anything or rewrites the public `wolvic` branch — that final step is
# deliberately left out and gated on explicit human action (see wolvic/REBASE.md).

set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "$HERE/lib.sh"

SRC_BRANCH="${SRC_BRANCH:-$WOLVIC_BRANCH}"

usage() { sed -n '2,30p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit "${1:-0}"; }

# ---------------------------------------------------------------------------------------------
cmd_detect_base() {
  local build base count files
  build="$(current_build)"
  log "chrome/VERSION build = $build  -> old branch-heads/$build"
  verify_branch_head "$build" >/dev/null && log "branch-heads/$build resolves upstream"

  if ! git rev-parse --verify --quiet "$REF_OLD" >/dev/null; then
    warn "$REF_OLD not fetched yet; run 'fetch' to compute OLD_BASE precisely."
    return 0
  fi
  base="$(old_base)"
  count="$(git rev-list --count "$base".."$SRC_BRANCH")"
  files="$(git diff --name-only "$base".."$SRC_BRANCH" | wc -l | tr -d ' ')"
  log "OLD_BASE   = $base"
  log "            $(git log -1 --format='%h %s' "$base")"
  log "replaying  = $count commits, $files files  (from $SRC_BRANCH)"
  # Guard against a wrong base (e.g. accidental diff vs origin/main = ~300k files).
  if (( files > 5000 )); then
    die "OLD_BASE looks wrong: $files files changed (expected hundreds). Investigate before rebasing."
  fi
}

cmd_resolve_target() {
  local arg="${1:?usage: resolve-target <milestone|branch-build>}"
  local build sha
  build="$(resolve_build "$arg")"
  sha="$(verify_branch_head "$build")"
  log "target '$arg' -> branch-heads/$build  ($sha)"
  echo "$build"
}

cmd_fetch() {
  local arg="${1:?usage: fetch <milestone|branch-build>}"
  local old_build new_build
  old_build="$(current_build)"
  new_build="$(resolve_build "$arg")"
  verify_branch_head "$old_build" >/dev/null
  verify_branch_head "$new_build" >/dev/null
  log "fetching old branch-heads/$old_build -> $REF_OLD"
  git fetch --no-tags "$CHROMIUM_SRC_URL" "+refs/branch-heads/${old_build}:${REF_OLD}"
  log "fetching new branch-heads/$new_build -> $REF_NEW"
  git fetch --no-tags "$CHROMIUM_SRC_URL" "+refs/branch-heads/${new_build}:${REF_NEW}"
  log "OLD_BASE = $(old_base)"
  log "NEW_BASE = $(new_base)  (branch-heads/$new_build)"
}

cmd_report() {
  local arg="${1:-}"
  [[ -n "$arg" ]] && cmd_fetch "$arg" >/dev/null 2>&1 || true
  exec "$HERE/api-drift-report.sh"
}

cmd_onto() {
  local arg="${1:?usage: onto <milestone|branch-build> [--yes]}"; shift || true
  local yes=0; [[ "${1:-}" == "--yes" ]] && yes=1
  local base new new_build target
  base="$(old_base)"; new="$(new_base)"
  new_build="$(resolve_build "$arg")"
  target="wolvic_update_m$( [[ "$arg" =~ ^[0-9]{1,3}$ ]] && echo "$arg" || echo "$new_build" )"

  log "Will replay $SRC_BRANCH ($(git rev-list --count "$base".."$SRC_BRANCH") commits) onto branch-heads/$new_build."
  log "git checkout -B $target $SRC_BRANCH"
  log "git rebase --onto $new $base $target"
  if (( ! yes )); then
    warn "dry-run. Re-run with --yes to start. Generate the drift report first if you haven't:"
    warn "    $HERE/api-drift-report.sh"
    return 0
  fi
  git checkout -B "$target" "$SRC_BRANCH"
  git rebase --onto "$new" "$base" "$target" || {
    warn "rebase stopped on a conflict. For the conflicting file, consult the drift report and run:"
    warn "    $HERE/migrate-apis.sh --file <path>"
    warn "then 'git add' + 'git rebase --continue'. A conflict in a revert => re-test the original bug."
    return 1
  }
  log "rebase complete: $target is $SRC_BRANCH replayed onto branch-heads/$new_build"
  log "the original $WOLVIC_BRANCH is untouched (it is your backup)."
}

cmd_status() {
  echo "src branch    : $SRC_BRANCH"
  echo "current build : $(current_build)"
  for r in "$REF_OLD" "$REF_NEW"; do
    if git rev-parse --verify --quiet "$r" >/dev/null; then
      echo "$r : $(git rev-parse --short "$r")"
    else
      echo "$r : (not fetched)"
    fi
  done
  git rev-parse --verify --quiet "$REF_OLD" >/dev/null && echo "OLD_BASE      : $(old_base)"
  git rev-parse --verify --quiet "$REF_NEW" >/dev/null && echo "NEW_BASE      : $(new_base)"
}

# ---------------------------------------------------------------------------------------------
main() {
  local sub="${1:-}"; shift || true
  case "$sub" in
    detect-base)    cmd_detect_base "$@" ;;
    resolve-target) cmd_resolve_target "$@" ;;
    fetch)          cmd_fetch "$@" ;;
    report)         cmd_report "$@" ;;
    onto)           cmd_onto "$@" ;;
    status)         cmd_status "$@" ;;
    -h|--help|help|"") usage 0 ;;
    *) die "unknown subcommand '$sub' (try --help)" ;;
  esac
}
main "$@"
