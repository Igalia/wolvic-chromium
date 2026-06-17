#!/usr/bin/env bash
# api-drift-report.sh — detect upstream API drift that will break the Wolvic patches.
#
# Front-runs the slow RBE build with a static analysis of what upstream changed between the old
# and the new milestone *underneath the files Wolvic patches*, plus drift in the upstream headers
# the self-contained wolvic/ module includes.
#
# Produces two artifacts from a single scan so they never disagree:
#   drift-report-m<NEW_BUILD>.md    human-readable, grouped, BREAKING summary on top
#   drift-report-m<NEW_BUILD>.json  machine-actionable, one object per affected file
#                                   (consumed by migrate-apis.sh)
#
# Requires `fetch` to have populated the rebase refs (run rebase-wolvic.sh fetch <target> first).

set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib.sh
source "$HERE/lib.sh"

OUT_DIR="${OUT_DIR:-$HERE}"

OLD_BASE="$(old_base)"
NEW_BASE="$(new_base)"
# The branch-heads number is the BUILD component of chrome/VERSION at NEW_BASE.
NEW_BUILD="$(git show "$NEW_BASE:chrome/VERSION" 2>/dev/null | awk -F= '/^BUILD=/{print $2}')"
[[ -n "$NEW_BUILD" ]] || NEW_BUILD="$(git rev-parse --short "$NEW_BASE")"

MD="$OUT_DIR/drift-report-m${NEW_BUILD}.md"
JSON="$OUT_DIR/drift-report-m${NEW_BUILD}.json"

log "OLD_BASE=$OLD_BASE  NEW_BASE=$NEW_BASE  -> m$NEW_BUILD"
log "scanning... (this is read-only git; no build)"

OLD_BASE="$OLD_BASE" NEW_BASE="$NEW_BASE" WOLVIC_BRANCH="$WOLVIC_BRANCH" \
MD="$MD" JSON="$JSON" NEW_BUILD="$NEW_BUILD" ROOT="$(repo_root)" python3 - <<'PY'
import json, os, re, subprocess, sys

OLD = os.environ["OLD_BASE"]; NEW = os.environ["NEW_BASE"]
WOLVIC = os.environ["WOLVIC_BRANCH"]
MD = os.environ["MD"]; JSON = os.environ["JSON"]; NEW_BUILD = os.environ["NEW_BUILD"]
ROOT = os.environ["ROOT"]

def git(*args):
    return subprocess.run(["git", *args], capture_output=True, text=True).stdout

# ---- classify a path into one of the rebase feature groups --------------------------------
GROUPS = [
    ("gpu-reverts",        r"(gpu/ipc/service/image_transport_surface|ui/gl/(gl_surface|presenter|dcomp_presenter)|skia_output_surface|gpu/command_buffer/service/feature_info)"),
    ("xr-vr",              r"(content/browser/xr/|device/vr/|/vr/|xr_device\.mojom)"),
    ("webpayments",        r"(payment|PaymentRequest|onCreateNewPaymentHandler)"),
    ("webauthn",           r"webauthn"),
    ("autofill-password",  r"(autofill|password)"),
    ("mediasession",       r"(MediaSession|media_session|media/session)"),
    ("mojo-bindings",      r"mojo/public/java/"),
    ("input-ui",           r"(SelectionPopupController|SelectPopup|DateTimeChooser|AndroidFontLookup|ContentView)"),
    ("threadchecker-reverts", r"(ThreadUtils|FrameMetricsStore|jank_tracker)"),
    ("fieldtrial-reverts", r"fieldtrial_testing_config"),
    ("tab-find-in-page",   r"(/Tab\.java|tab_jni|FindResult|find_in_page|WolvicWebContentsDelegate)"),
    ("build-glue",         r"(^DEPS$|/BUILD\.gn$|rules\.gni|proguard|\.flags$)"),
    ("core-module",        r"wolvic/"),
]
def group_of(path):
    for name, rx in GROUPS:
        if re.search(rx, path):
            return name
    return "uncategorized"

# ---- old-side line ranges of a diff (both base..wolvic and base..new share OLD as old side) --
HUNK = re.compile(r"^@@ -(\d+)(?:,(\d+))? \+\d+(?:,\d+)? @@(.*)")
def old_ranges_and_ctx(diff):
    ranges, ctx = [], []
    for ln in diff.splitlines():
        m = HUNK.match(ln)
        if m:
            start = int(m.group(1)); length = int(m.group(2) or 1)
            ranges.append((start, start + max(length, 1)))
            c = m.group(3).strip()
            if c:
                ctx.append(c)
    return ranges, ctx
def overlap(a, b):
    return any(s1 < e2 and s2 < e1 for s1, e1 in a for s2, e2 in b)

# ---- symbols our patch touches (function/method context from hunk headers + def-ish + lines) -
SYM = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]+)\s*\(")
def symbols(diff):
    out = set()
    for ln in diff.splitlines():
        if ln.startswith("@@"):
            for m in SYM.finditer(ln): out.add(m.group(1))
        elif ln.startswith(("+", "-")) and not ln.startswith(("+++", "---")):
            body = ln[1:]
            if re.search(r"\b(void|public|private|protected|static|virtual|override|class|interface)\b", body):
                for m in SYM.finditer(body): out.add(m.group(1))
    out.discard("if"); out.discard("for"); out.discard("while"); out.discard("switch")
    return sorted(out)[:25]

entries = []

# ============ 1. modified-upstream files (the dangerous set) =================================
for ln in git("diff", "--name-status", "-M", f"{OLD}..{WOLVIC}").splitlines():
    parts = ln.split("\t")
    if not parts or not parts[0].startswith("M"):
        continue
    path = parts[-1]
    if path.startswith("wolvic/") or path.startswith("out/"):
        continue
    # what upstream did to this file between OLD and NEW
    up_ns = git("diff", "--name-status", "-M", OLD, NEW, "--", path).strip()
    up_status, renamed_to, severity = "unchanged", None, "ok"
    our_diff = git("diff", OLD, WOLVIC, "--", path)
    up_diff = ""
    if up_ns:
        f = up_ns.split("\t")
        code = f[0]
        if code.startswith("R"):
            up_status, renamed_to, severity = "R", f[-1], "breaking"
            up_diff = git("diff", "-M", OLD, NEW, "--", path)
        elif code.startswith("D"):
            up_status, severity = "D", "breaking"
        elif code.startswith("M"):
            up_status = "M"
            up_diff = git("diff", OLD, NEW, "--", path)
            our_r, _ = old_ranges_and_ctx(our_diff)
            up_r, _ = old_ranges_and_ctx(up_diff)
            severity = "review" if overlap(our_r, up_r) else "ok"
    entries.append({
        "wolvic_file": path,
        "group": group_of(path),
        "kind": "modified-upstream",
        "upstream_status": up_status,
        "renamed_to": renamed_to,
        "severity": severity,
        "symbols": symbols(our_diff),
        "upstream_diff": up_diff,
        "wolvic_patch": our_diff,
    })

# ============ 2. header-dependency drift (our self-contained module's includes) ==============
INC = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.M)
UP_PREFIX = ("content/", "components/", "ui/", "gpu/", "base/", "media/", "device/",
             "services/", "net/", "mojo/", "third_party/blink/")
seen_hdr = set()
wolvic_cc = git("ls-files", "wolvic/").splitlines()
for src in wolvic_cc:
    if not src.endswith((".cc", ".h", ".mm", ".cpp")):
        continue
    try:
        with open(os.path.join(ROOT, src), encoding="utf-8", errors="ignore") as fh:
            text = fh.read()
    except OSError:
        continue
    for m in INC.finditer(text):
        hdr = m.group(1)
        if not hdr.startswith(UP_PREFIX) or hdr in seen_hdr:
            continue
        seen_hdr.add(hdr)
        ns = git("diff", "--name-status", "-M", OLD, NEW, "--", hdr).strip()
        if not ns:
            continue  # header unchanged across the jump
        f = ns.split("\t"); code = f[0]
        st = "D" if code.startswith("D") else ("R" if code.startswith("R") else "M")
        entries.append({
            "wolvic_file": hdr,
            "group": group_of(hdr),
            "kind": "header-dep",
            "upstream_status": st,
            "renamed_to": f[-1] if st == "R" else None,
            "severity": "breaking" if st in ("D", "R") else "review",
            "symbols": [],
            "included_by": src,
            "upstream_diff": git("diff", "-M", OLD, NEW, "--", hdr) if st != "D" else "",
            "wolvic_patch": "",
        })

# ---- write JSON ----------------------------------------------------------------------------
rank = {"breaking": 0, "review": 1, "ok": 2}
entries.sort(key=lambda e: (rank.get(e["severity"], 9), e["group"], e["wolvic_file"]))
with open(JSON, "w") as fh:
    json.dump({"old_base": OLD, "new_base": NEW, "milestone_build": NEW_BUILD,
               "entries": entries}, fh, indent=2)

# ---- write Markdown ------------------------------------------------------------------------
def by_sev(s): return [e for e in entries if e["severity"] == s]
with open(MD, "w") as fh:
    w = fh.write
    w(f"# Wolvic API-drift report — onto branch-heads/{NEW_BUILD}\n\n")
    w(f"- OLD_BASE `{OLD[:12]}` → NEW_BASE `{NEW[:12]}`\n")
    w(f"- {len(by_sev('breaking'))} breaking, {len(by_sev('review'))} review, "
      f"{len(by_sev('ok'))} ok  (total {len(entries)} files)\n\n")

    w("## ⚠️ BREAKING — upstream deleted/renamed a file we patch or include\n\n")
    b = by_sev("breaking")
    if not b:
        w("_None._\n\n")
    for e in b:
        rn = f" → renamed to `{e['renamed_to']}`" if e["renamed_to"] else " (deleted)"
        w(f"- **`{e['wolvic_file']}`**{rn}  _[{e['group']}, {e['kind']}]_\n")
    w("\n")

    w("## REVIEW — upstream edited the same region we patch (or a changed header we include)\n\n")
    r = by_sev("review")
    if not r:
        w("_None._\n\n")
    cur = None
    for e in r:
        if e["group"] != cur:
            cur = e["group"]; w(f"\n### {cur}\n\n")
        syms = ", ".join(e["symbols"][:8])
        w(f"- `{e['wolvic_file']}` ({e['kind']})"
          + (f" — our symbols: {syms}" if syms else "") + "\n")
    w("\n")

    w("## OK — upstream changed elsewhere in the file, no overlap with our hunks\n\n")
    o = by_sev("ok")
    w(f"_{len(o)} files_ (see JSON for the list).\n\n")

    w("---\n")
    w("Per-file upstream diffs and our patches are in the JSON. To get migration suggestions:\n\n")
    w("```\nwolvic/tools/rebase/migrate-apis.sh            # all breaking+review\n")
    w("wolvic/tools/rebase/migrate-apis.sh --file <path>\n```\n")

print(f"wrote {MD}")
print(f"wrote {JSON}")
PY

log "done."
