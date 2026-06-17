# Rebasing Wolvic-Chromium onto a new Chromium milestone

This is the runbook for moving the Wolvic changes onto a newer Chromium release. The mechanical
steps are scripted under `wolvic/tools/rebase/`; only conflict resolution and on-device testing
are manual. It is generic — it does not assume any particular source or target milestone.

## TL;DR

```bash
cd <chromium-src>                       # the wolvic checkout (run all commands from the repo root)
T=<milestone>                           # target, e.g. 150 — or a branch-heads build number

# INCREMENTAL HOPS: if you are replaying a previous hop's branch (not the original `wolvic`),
# export BOTH vars so the base is computed from that branch's fork point, not M124's:
#   export WOLVIC_BRANCH=wolvic_update_m<prev>   # used to compute OLD_BASE = merge-base(REF_OLD, this)
#   export SRC_BRANCH=wolvic_update_m<prev>      # the branch actually replayed
# Setting only SRC_BRANCH is a trap: OLD_BASE then resolves to the M124 fork point and detect-base
# reports ~100k files ("OLD_BASE looks wrong"). Set WOLVIC_BRANCH too.

wolvic/tools/rebase/rebase-wolvic.sh resolve-target $T   # confirm it maps to a branch-heads ref
# Eyeball that number against https://chromiumdash.appspot.com/releases — a branch-heads ref
# exists for every build number, so make sure it's the milestone you intended.
wolvic/tools/rebase/rebase-wolvic.sh fetch        $T     # fetch old+new bases, compute OLD/NEW_BASE
wolvic/tools/rebase/rebase-wolvic.sh detect-base         # sanity-check the fork point & commit count
wolvic/tools/rebase/api-drift-report.sh                  # what upstream changed under our patches
wolvic/tools/rebase/rebase-wolvic.sh onto $T --yes       # the actual rebase
# resolve conflicts (see Step 3), then resolve DEPS, gclient sync, build, test
```

The final step that **replaces the public `wolvic` branch is intentionally not scripted** — see
[Switching the branch](#switching-the-branch-gated).

## Background

- **Pick the target.** Prefer an **LTS milestone** when one is available — it gets security
  backports for longer, so you rebase less often. Check
  <https://chromiumdash.appspot.com/schedule> for the milestone whose LTR window is furthest out.
  `resolve-target` accepts either a milestone (`150`) or an explicit branch-heads build (`7871`).
- **Remotes.** `origin` already points at `https://chromium.googlesource.com/chromium/src`; you do
  not need a separate `chromium` remote. The `wolvic` remote is the Igalia fork.
- **The fork point is derived, not memorised.** The `BUILD` component of `chrome/VERSION` is the
  current (old) branch-heads number; `OLD_BASE` is `git merge-base refs/branch-heads/<BUILD> wolvic`.
  `fetch`/`detect-base` compute this for you and abort if it looks wrong.
- **Reason against `OLD_BASE`, never `origin/main`.** A diff against `origin/main` shows hundreds of
  thousands of files (all of upstream's progress); the actual Wolvic delta against `OLD_BASE` is a
  few hundred files. The scripts always use `OLD_BASE`.

## Step 1 — Resolve, fetch, sanity-check

```bash
T=150
wolvic/tools/rebase/rebase-wolvic.sh resolve-target $T   # -> branch-heads/<build>
wolvic/tools/rebase/rebase-wolvic.sh fetch $T            # writes refs/wolvic-rebase/{old,new}-base
wolvic/tools/rebase/rebase-wolvic.sh detect-base         # aborts if the fork point looks wrong
```

The `fetch` of a target many milestones ahead can be a large download; it is read-only and changes
nothing in your tree or branches.

## Step 2 — Drift report (do this before touching conflicts)

```bash
wolvic/tools/rebase/api-drift-report.sh
```

Produces, next to the script:

- `drift-report-m<build>.md` — read the **BREAKING** section first (upstream files we patch or
  include that were deleted/renamed), then **REVIEW** (upstream edited the same region we patch, or
  a header we include changed).
- `drift-report-m<build>.json` — the machine-readable version that drives migration.

This front-runs the multi-hour build: it tells you where the API moved before the compiler does.

## Step 3 — The rebase

```bash
wolvic/tools/rebase/rebase-wolvic.sh onto $T            # dry-run: prints the exact git command
wolvic/tools/rebase/rebase-wolvic.sh onto $T --yes      # creates wolvic_update_m<T> and rebases
```

Internally: `git rebase --onto <NEW_BASE> <OLD_BASE> <SRC_BRANCH>` (default `SRC_BRANCH=wolvic`).
Your original `wolvic` branch is left untouched as a backup. When the rebase stops on a conflict:

```bash
wolvic/tools/rebase/migrate-apis.sh --file <conflicting/path>
```

…asks Claude to propose an adapted patch (written to `suggestions/`, **not** auto-applied). Review
it, fix the file, `git add`, `git rebase --continue`. You can also generate suggestions for the
whole report up front with `migrate-apis.sh`, or `migrate-apis.sh --apply` to drop them into the
working tree unstaged.

**Watch for revert conflicts.** Wolvic carries some reverts of upstream commits. If applying one
conflicts, upstream very likely changed or fixed the thing the revert worked around — re-test the
original issue and consider dropping the revert instead of forcing it through.

## Step 4 — DEPS, sync, build, test

1. **Resolve `DEPS`.** `DEPS` is a patched-upstream file, so it usually conflicts. Re-apply the
   Wolvic edits (notably any custom dependency pins — see [Custom forks](#custom-dependency-forks)).
2. `gclient sync`
3. Build:
   ```bash
   autoninja -C out/android content_shell_resources_grit content_aar ui_aar
   ```
   Uses remote execution (`use_remoteexec=true`); it takes minutes-to-hours — slowness is not
   failure. If it fails on RBE misconfiguration, fix the RBE setup rather than the code. See
   <https://chromium.googlesource.com/chromium/src/+/HEAD/docs/linux/build_instructions.md#use-remote-execution>.
4. Confirm `out/android/Content.aar` and `out/android/ChromiumUi.aar` regenerate, then test in
   Wolvic on a device — especially anything whose patch conflicted during the rebase.

## Runtime failures & accumulated gotchas

Compiling clean is not the finish line — milestones routinely break at *runtime* even when the build
is green: flipped GN-arg defaults, generated Java classes missing from the AAR, or a dropped revert
that only bites one specific feature. That per-failure-mode knowledge, plus the device-crash
debugging workflow (symbolizing tombstones, the stale-tombstone `BuildId` check), is collected in
[`REBASE-AGENTS.md`](REBASE-AGENTS.md) — kept separate so this runbook stays a clean process doc.
Skim it before the on-device test in Step 4, and add to it whatever the next hop teaches.

## Switching the branch (gated)

Only after the build is green and tested on device, and only with explicit sign-off. **If you are
migrating incrementally across several milestones, do this once — at the final milestone — not per
hop.** Keep each intermediate `wolvic_update_m<N>` branch around; they are useful anchors for
comparing releases. Replace `<OLD>`/`<NEW>` with the milestones involved:

```bash
git tag wolvic_m<OLD> wolvic              # tag the OLD base milestone
git push wolvic wolvic_m<OLD>             # back it up on the remote
# confirm wolvic_update_m<NEW> contains everything and nothing local is unsaved
git checkout wolvic
git reset --hard wolvic_update_m<NEW>
git push -f wolvic wolvic                 # replaces the public branch — point of no easy return
```

Do not run the force-push without that sign-off.

## Custom dependency forks

Wolvic may pin **forks of dependencies** in `DEPS` (for example a fork of ANGLE under
`github.com/Igalia/...`). On every rebase, for each such fork:

1. **Question whether it is still needed.** A fork usually carries a small patch added for one past
   problem; upstream may have since fixed or obviated it. Re-test the feature the fork exists for
   against the *stock* upstream dependency at the new milestone.
2. **If no longer needed:** point `DEPS` back at the upstream dependency at the revision the new
   milestone pins, and drop the fork's overrides. One less thing to maintain.
3. **If still needed:** rebase the fork's few patches onto the new milestone's base in that fork's
   own checkout, push the new branch, and update its revision pin in `DEPS`.

Either way, run `gclient sync` afterwards.

## Large jumps (optional curation)

A normal single-milestone rebase just replays the Wolvic commits as-is. For an unusually large jump
(many milestones at once), resolving conflicts on long-stale individual commits can be more painful
than re-applying the current state of each feature. If so, you can **curate the series first** with
an ad-hoc interactive rebase before running `onto`:

```bash
git checkout -b wolvic-curated wolvic
git rebase -i <OLD_BASE>        # squash/fixup commits into a handful of per-feature patches
git diff wolvic wolvic-curated  # MUST be empty — proves curation dropped nothing
SRC_BRANCH=wolvic-curated wolvic/tools/rebase/rebase-wolvic.sh onto $T --yes
```

This is a one-off convenience, not part of the standard flow — keep each revert as its own patch so
its conflict signal (see Step 3) survives.

## Tooling reference

| File | Purpose |
|------|---------|
| `tools/rebase/rebase-wolvic.sh` | orchestrator: `detect-base`, `resolve-target`, `fetch`, `report`, `onto`, `status` |
| `tools/rebase/api-drift-report.sh` | dual-output (`.md` + `.json`) upstream API-drift report |
| `tools/rebase/migrate-apis.sh` | turns the drift JSON into proposed patches via `claude -p` |
| `tools/rebase/lib.sh` | shared base/milestone-resolution helpers |
