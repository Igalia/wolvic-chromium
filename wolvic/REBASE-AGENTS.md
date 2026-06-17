# Wolvic-Chromium rebase — operational notes for agents

Companion to [`REBASE.md`](REBASE.md). `REBASE.md` is the human runbook: the process and the
commands for moving Wolvic onto a new Chromium milestone. **This file is the accumulated
failure-mode knowledge** — the things that compile clean and then break at runtime, plus the
debugging workflow — written to bootstrap an agent (or anyone) doing the migration. Append new
gotchas here as each hop teaches them; keep `REBASE.md` a clean process doc.

## Adapting to a changed/new API parameter — check `android_webview/` first, and ask if non-trivial

When upstream adds or changes a parameter on an API Wolvic implements or calls (a new ctor arg, a
new virtual param, a renamed method), **before inventing a value, look at what
`chromium/android_webview/` passes/does with it.** `android_webview` is a *real, production
embedder* (unlike `content_shell`, which is a test harness), so its handling is usually the most
relevant template for Wolvic — another XR/standalone embedder. Grep the webview tree for the symbol,
e.g. `git grep -nA6 'NewApiName' android_webview/`, and prefer mirroring its default over a
guessed `null`/`0`.

**This is a judgment call, not a mechanical copy.** Two outcomes:

- *Obvious & cheap* — the new param has a benign default (pass `null`, an empty `FilePath`, an
  existing accessor like `client.GetChannelForVariations()`) and adopting webview's value changes
  no behaviour Wolvic relies on. Just do it (and note it on the runtime watch-list if behaviour
  *could* shift — e.g. a visibility/surface arg).
- *Non-trivial — STOP AND ASK.* If actually using the parameter the way `android_webview` does
  would mean implementing a new API surface, wiring a new delegate, or adding a subsystem Wolvic
  doesn't have, that is a **product decision, not a rebase mechanic.** Surface it to the maintainer
  with the webview reference and the cost, rather than silently stubbing it or silently porting
  webview's full implementation. A wrong default here compiles clean and misleads.

(Worked example, M132: `VariationsSeedStore`/`VariationsSafeSeedStoreLocalState` gained
`channel` + `seed_file_dir`. `android_webview/browser/aw_feature_list_creator.cc` and
`content/shell/.../shell_content_browser_client.cc` both pass
`client.GetChannelForVariations()` / `GetVariationsSeedFileDir()` — benign accessors already
available on our `VariationsServiceClient`, so mirroring them was the obvious, no-new-API choice.)

## Runtime GN-arg defaults (check every milestone)

A milestone can compile cleanly yet fail at runtime because Chromium flipped a GN-arg *default*.
These live in your build's `args.gn` (e.g. `out/android/args.gn`) and must be **persisted in
whatever generates that file** — a clean `gn gen` wipes anything not in your build-args source.

M128 required these (each was a default flip that broke the AAR at runtime):

```gn
enable_jni_multiplexing = false    # M128 turned multiplexing on for release; the Gradle-consumed
                                   # AAR ships the readable org.jni_zero.GEN_JNI -> UnsatisfiedLinkError
use_v8_context_snapshot = false    # we ship snapshot_blob_64.bin, not v8_context_snapshot.bin
include_both_v8_snapshots = false  # else content adds --use-context-snapshot to the renderer and
                                   # shares a context-snapshot FD we don't ship -> renderer aborts in
                                   # OpenV8File (sandbox can't read the APK) -> blank/transparent web
```

When a build runs but shows blank web content or crashes a child process, suspect these before the
code. (Official builds suppress INFO/VLOG: pass `--enable-logging --v=1 --vmodule=...` via the
engine command line, and symbolize native crashes against `out/android/lib.unstripped/*.so`.)

## Build won't start: autoninja injects siso flags this checkout rejects

When an agent runs the build, `autoninja` detects the AI-agent env (`CLAUDECODE`/`AI_AGENT` set)
and **prepends `--quiet --batch=false --heartbeat_period=30s`** to the siso invocation. An older
siso (as pinned by some milestones, e.g. M132) doesn't define `--quiet`/`--heartbeat_period`, so
siso dumps its flag usage and exits with *"The build has finished with an error"* — before
compiling anything. It looks like a build failure but is purely a wrapper/siso-version mismatch.
Fix: strip the env detection for the invocation:

```bash
env -u CLAUDECODE -u AI_AGENT autoninja -C out/android content_shell_resources_grit content_aar ui_aar
```

(Also: piping the build through `| tail` masks the real exit code — `tail` returns 0 even when the
build failed. Grep the log for `FAILED:`/`finished with an error` instead of trusting `$?`.)

## Fixing the Wolvic compile drift efficiently (`-k 0`, batch by file)

siso stops at the first failed step by default (`-k 1`), so a naive rebuild surfaces **one file's
errors per multi-minute cycle**. Once `gn gen` passes and you're into compilation, rebuild with
**`-k 0`** (keep going until ∞ fail) to compile every Wolvic translation unit and surface the whole
remaining error set at once, then batch-fix:

```bash
env -u CLAUDECODE -u AI_AGENT autoninja -C out/android -k 0 content_shell_resources_grit content_aar ui_aar
```

Enumerate the full set from `out/android/siso_output` (NOT the piped `tail`, which truncates):
`grep -E 'FAILED:|error:' out/android/siso_output`. To iterate on a single file fast, build just its
object: `autoninja -C out/android obj/wolvic/libcontent_native/<file>.o`. A header error (e.g. a
changed `wolvic_autofill_client.h`) shows up as failures in every `.cc` that includes it — fix the
header and they all clear. Compile-green is not done: `libcontent_native.so` only links after all
objects compile, so removed/renamed methods can still surface as link errors in the SOLINK step.

## Heavy autofill / password-client churn — stub it, and delegate

`wolvic_autofill_client`, `wolvic_autofill_manager`, and `wolvic_password_manager_client` implement
big upstream abstract interfaces (`AutofillClient`, `AutofillManager`, `PasswordManagerClient`) that
churn most milestones (renamed/added/removed pure virtuals, changed return types). Wolvic's impls
are intentionally **no-op stubs** — Wolvic ships no real autofill/password UI. So: implement every
new/changed pure virtual as a minimal stub (return `{}`/`nullptr`/`false`/default id, empty void),
align signatures exactly to the M-base, and delete overrides the base no longer declares. This is
self-contained, well-scoped work — good to hand to a sub-agent that iterates with single-object
compiles. M128 and M132 both needed a pass here.

## `WebDataServiceWrapper` requires an `OSCryptAsync*` (M132+)

`components/webdata_services/web_data_service_wrapper.h`'s ctor gained a mandatory
`os_crypt_async::OSCryptAsync* os_crypt` (plus `bool use_in_memory_autofill_account_database`).
**`nullptr` crashes** — `WebDatabaseService::LoadDatabase` dereferences it (`os_crypt->GetInstance`).
`android_webview` (AwBrowserProcess) creates an `OSCryptAsync` with an **empty key-provider list**
(encryption delegated to OSCrypt) and owns it at browser-process lifetime. Wolvic has no
browser-process object; the lightest correct option is a function-local
`static base::NoDestructor<os_crypt_async::OSCryptAsync> os_crypt(std::vector<std::pair<size_t,
std::unique_ptr<os_crypt_async::KeyProvider>>>{});` in `web_data_service_factory.cc`'s
`BuildWebDataService` (process lifetime, no plumbing). Note the `{}` not `()` on the vector —
most-vexing-parse otherwise. Add a `//components/os_crypt/async/browser` dep in `wolvic/BUILD.gn`.

## Runtime class packaging (AAR `jar_included_patterns`)

A milestone often references *new* generated Java classes (mojom bindings, etc.) that compile fine
(they exist in the tree) but aren't in the AAR, so they fail at runtime with
`NoClassDefFoundError` / `ClassNotFoundException` — only when the specific code path runs. The
compiler and the drift report can't catch these. Fix: add the missing package wildcard to
`jar_included_patterns` in `wolvic/BUILD.gn`.

To avoid one rebuild per missing class, **read the IDL of the failing message** and add the whole
cluster at once. M128 examples (all in the IME `UpdateCursorAnchorInfo` path):
`org/chromium/window/*.class`, and — from `third_party/blink/public/mojom/input/ime_host.mojom`'s
imports — `org/chromium/gfx/mojom/*.class` (gfx.mojom.Rect) and `org/chromium/skia/mojom/*.class`
(skia.mojom.SkColor). Expect a few more as you exercise payments/permissions/media.

## A milestone bump can expose latent re-entrancy/lifetime bugs in *unchanged* fork code

A clean rebuild and a passing 2D test don't mean the fork's own C++ is safe — upstream may drive a
path *differently* at the new milestone and trip a latent bug in code the rebase never touched.
Worked example (M132): a specific WebXR page crashed 100% (others fine) with a deterministic SIGSEGV
in `WolvicPermissionManager::CompleteRequest`. The lifecycle code was byte-identical to M128; M132
just exercises the XR permission flow such that a permission callback re-enters the manager while the
request is still mid-completion. `CompleteRequest` ran callbacks while the `InProgressRequest` was
still in `in_progress_requests_` (iterating its `callbacks` by reference) and erased afterwards — so a
re-entrant request found, mutated, or erased the in-flight request → use-after-free. Fix: move the
`unique_ptr` out of the list and erase the slot *before* running callbacks (the standard Chromium
"extract then notify" idiom). When the build is green but a feature crashes, suspect this; it is not
necessarily in your diff.

Diagnosis method that cracked it (generalizable):
- Match the tombstone `BuildId` to `out/android/lib.unstripped/libcontent_native.so`, then
  `llvm-symbolizer -e … -f -C -i <pc>` the top frames.
- **Judge determinism by reproducing 2–3×.** Identical frames + a *fixed* fault offset (here
  `+0x270`) with a *varying* garbage base = dereferencing a wild pointer at a constant field offset →
  a deterministic logic/lifetime bug at that site. Varying frames/offset = random heap corruption
  sourced elsewhere (then audit recent diffs for an out-of-bounds write instead).
- A *disengaged* `std::optional`/callback whose destructor faults is a corruption canary, not the
  culprit — the object's memory was already bad; look at who freed/overwrote it.

## On-device testing: confirm you're running the build you think you are

Before debugging a device symptom, verify the headset is running your *fresh* build, not a stale or
different one — this wastes the most time. Checks: `adb shell dumpsys package <pkg> | grep -E
'versionName|lastUpdateTime'` (is the install timestamp from this build?), and `adb shell ps -A |
grep wolvic` + the child-process type (a `…:tabN` / `org.mozilla.gecko.*` child = a **Gecko** build,
not the Chromium fork). Wolvic builds install per flavor under distinct app-ids (`com.igalia.wolvic`
vs `com.igalia.wolvic.dev`). The **aosp** flavor on a Pico renders blank with `FrameEvents:
updateAcquireFence: Did not find frame` spam (no Pico OpenXR integration) — build/install the
**picoxr** flavor for Pico hardware. (`adb logcat`'s dedicated `-b crash` buffer holds tombstones and
is *not* flooded by the `FrameEvents` spam that buries them in the main buffer.)

Disk note: `/tmp` here is a small (~920M) partition that fills with `siso.*.log.*` build logs (a
build run can leave 500M+ across dozens of files) and fails the build with
`OSError: [Errno 28] No space left on device`. `rm -f /tmp/siso.*.log.*` is safe — siso writes a
fresh log each run.

## Debugging device crashes

- **Confirm the tombstone is from the build you think it is.** Compare the tombstone's `BuildId`
  against `llvm-readelf -n out/android/lib.unstripped/libcontent_native.so | grep "Build ID"`. A
  mismatch (or a repeated PID) means logcat replayed a *stale* tombstone or the device runs an old
  APK — don't chase a bug you already fixed. After a rebuild the BuildId changes.
- **Know which `.so` crashed.** `libcontent_native.so` is *our* Chromium AAR (this repo). The Wolvic
  app's own `crow::` OpenXR/rendering code is `libnative-lib.so` — a **separate repo**; crashes there
  are out of scope for the rebase.
- Symbolize: `third_party/llvm-build/Release+Asserts/bin/llvm-symbolizer -e
  out/android/lib.unstripped/libcontent_native.so -f -C -i <offset>`.

## Dropping a revert? Test the feature it was *for* — not just adjacent ones

Wolvic carries reverts of upstream commits. Some conflict on rebase (covered in `REBASE.md` Step 3);
others **apply cleanly or get dropped silently** — and those are the dangerous ones, because nothing
forces you to think about them. Before declaring a dropped revert unneeded, test the *specific*
feature it protected, not whatever is convenient.

Worked example (M128): the "GPU/graphics reverts" group (incl. the revert of *Remove
ImageTransportSurfaceDelegate*) was dropped and validated against **2D web rendering** — which works,
because 2D uses the Android SurfaceControl presenter. But **WebXR** forces the *other* GPU surface
path (`can_be_used_with_surface_control=false` -> `ImageTransportSurface::CreateNativeGLSurface`),
and there upstream's `CreateNativeGLSurface` returns an already-initialized `NativeViewGLSurfaceEGL`
while `GLES2CommandBufferStub::Initialize` re-`Initialize()`s it. That issues a second
`eglCreateWindowSurface` on the same `ANativeWindow` -> `EGL_BAD_ALLOC`
(`native_window_api_connect: already connected`) -> `kSurfaceFailure` -> no frames -> passthrough.
The dropped revert had previously hidden this by wrapping the surface in a
`PassThroughImageTransportSurface` whose `Initialize` didn't re-issue the EGL call. Fix: a 1-line
wolvic patch in `gpu/ipc/service/gles2_command_buffer_stub.cc` (check `if (!surface_)` only, don't
re-`Initialize`) — far cheaper than re-adding the delegate machinery M128 re-architected away. On
the M150 path, check whether upstream dropped the redundant stub `Initialize` so the patch can retire.

Diagnostic that cracked it: `adb logcat -v threadtime` + `adb shell ps` to attribute the failing
`eglCreateWindowSurface` to a PID/process, then *count the BufferQueue connects* — two (success then
fail) on one surface with no disconnect meant one GLSurface initialized twice, not two surfaces.

**M132 update — this patch RETIRED.** By M132 upstream had independently made the same fix:
`GLES2CommandBufferStub::Initialize` now checks `if (!surface_)` only (no re-`Initialize()` of the
`CreateNativeGLSurface` result), with its own comment ("doubly initializing it can lead to errors").
During the M128→M132 rebase the Wolvic patch became an empty commit and was dropped. The lesson
holds in the other direction too: when the drift report flags a *Wolvic fix* file as
`modified-upstream`, check whether upstream adopted the same fix — if so, retire your patch instead
of forcing it through. Confirm by reading the target milestone's version of the function directly
(`git show <NEW_BASE>:path`), not just the conflict hunk.

## M136 API churn notes

### `AutofillClient` — 9 new pure virtuals, return-type changes, method removed

M136 added/changed substantially in `AutofillClient`:
- `GetAppLocale()` became `const std::string& GetAppLocale() const` (was not present).
- `GetVotesUploader()` new: returns `autofill::VotesUploader&` (reference, not pointer).
- `GetEntityDataManager()` new: returns `autofill::EntityDataManager*`.
- `GetSingleFieldFillRouter()` new: returns `autofill::SingleFieldFillRouter&` (reference).
- `IsAutofillEnabled/ProfileEnabled/PaymentMethodsEnabled()` all gained `const`.
- `DidFillForm(AutofillTriggerSource, bool is_refill)` added; `DidFillOrPreviewForm(ActionPersistence, …)` **removed** — delete the old override.
- `GetFormInteractionsUkmLogger()` returns `autofill::autofill_metrics::FormInteractionsUkmLogger&`.
  **Gotcha:** `autofill_metrics` is a *nested sub-namespace* inside `namespace autofill` (not a top-level namespace). From outside `namespace autofill` you must qualify it as `autofill::autofill_metrics::FormInteractionsUkmLogger`, not `autofill_metrics::FormInteractionsUkmLogger`.
- `GetCrowdsourcingManager()` and `GetPersonalDataManager()` changed from pointer to reference returns.
- `AutofillCrowdsourcingManager` ctor no longer takes a `LogManager*` arg.

### `ContentBrowserClient::CreateLoginDelegate` gained `GuestPageHolder*`

M136 added `content::GuestPageHolder* guest_page_holder` as the penultimate parameter (before the `LoginAuthRequiredCallback`). Add it to both the override declaration and definition.

### `BrowserPaymentRequest` — `getCertificateChain()` and `getDialogController()` added

Two new abstract Java methods:
- `byte @Nullable [][] getCertificateChain()` — use `org.chromium.build.annotations.Nullable` (not `androidx.annotation.Nullable`), which supports type-use annotations. Return `null`.
- `DialogController getDialogController()` — `DialogController` is a new interface in `org.chromium.components.payments`. Return a no-op anonymous class; `showLeavingIncognitoWarning` should call `approveCallback.run()` immediately (Wolvic has no incognito mode).

### `XRFrameData` restructured — `render_info` nested struct

`device::mojom::XRFrameData`'s `frame_id`, `views`, and `mojo_from_viewer` fields moved into a new `render_info` field of type `XRRenderInfo`. Initialize with `frame_data->render_info = device::mojom::XRRenderInfo::New()` before accessing them.

### `VRStageParameters::mojo_from_floor` renamed to `mojo_from_stage`

Global rename — `mojo_from_floor` → `mojo_from_stage`.

### `MailboxToSurfaceBridge` — `CreateSurface` and `ResizeSurface` removed

Both methods removed from the API. Drop all call sites in `wvr_manager.cc`.

### `absl::` → `std::` migration complete in M136

Any remaining `absl::optional`, `absl::variant`, `absl::holds_alternative`, `absl::get<>`, `absl::nullopt` must be migrated to `std::` equivalents. Replace `#include "third_party/abseil-cpp/absl/types/optional.h"` → `<optional>`, and similarly for `variant.h` → `<variant>`.

### `PasswordStoreBackend` interface trimmed

`GetAllLoginsForAccountAsync` and `RemoveLoginsByURLAndTimeAsync` removed. `RemoveLoginsCreatedBetweenAsync` gained a `base::OnceCallback<void(bool)> sync_completion` trailing param (pass-through/ignore is fine).

### `WebAuthnCredentialsDelegate::GetPasskeys()` return type changed

Old: `const std::optional<std::vector<PasskeyCredential>>&`
New: `base::expected<const std::vector<PasskeyCredential>*, PasskeysUnavailableReason>` — return `base::unexpected(PasskeysUnavailableReason::kNotReceived)` for the no-op stub. Include `base/types/expected.h`. `RetrievePasskeys` renamed to `RequestNotificationWhenPasskeysReady`; `NotifyForPasskeysDisplay()` added as a no-op.

### `AutofillAgent` constructor — options struct removed

M136 dropped the 6-bool options struct from `autofill::AutofillAgent`'s constructor. Drop the struct argument entirely.

### `AutofillManager` virtual method renames

- `OnTextFieldDidChangeImpl` → `OnTextFieldValueChangedImpl`
- `OnSelectControlDidChangeImpl` → `OnSelectControlSelectionChangedImpl`
- `OnJavaScriptChangedAutofilledValueImpl` dropped `bool formatting_only` parameter
- `OnLoadedServerPredictionsImpl(base::span<const raw_ptr<FormStructure, VectorExperimental>>)` added

### `embedder_support::BuildUserAgentFromOSAndProduct` moved namespace

Was `content::BuildUserAgentFromOSAndProduct`; now in `embedder_support::`. Include `components/embedder_support/user_agent_utils.h`.

### `TraceLog::SetProcessSortIndex` removed

`base::trace_event::TraceLog::GetInstance()->SetProcessSortIndex(…)` removed in M136 — delete the call site and the `base/trace_event/trace_log.h` include.

### `VariationsSafeSeedStoreLocalState` ctor reordered + new param

New order: `(local_state, seed_file_dir, channel, entropy_providers)`. Pass `nullptr` for `entropy_providers` if you're not using limited entropy mode.

### `SetUpFieldTrials` gained `EntropyProviders` arg

10th argument added: pass `*metrics_state_manager->CreateEntropyProviders(/*enable_limited_entropy_mode=*/false)`.

### `ActivityWindowAndroid` ctor gained `trackOcclusion`

5th boolean parameter added to the Java constructor. Pass `/* trackOcclusion= */ false`.

### `InstalledAppProviderImpl` ctor lost trailing `null`

Java constructor now takes 2 args instead of 3 — remove the trailing `null`.

### `WebContentsObserver` API changed (Java)

No-arg constructor + explicit `observe(webContents)` call + `webContentsDestroyed()` callback. Replace the 1-arg constructor pattern.

### `NOTREACHED_NORETURN()` removed

Renamed to `NOTREACHED()` — the old macro was deleted in M136.

## A patch that "drops a fork back to upstream" pins a stale revision (check DEPS after the rebase)

A Wolvic commit that retires a dependency fork (e.g. "Build against upstream ANGLE — drop the
wolvic-angle fork") typically hardcodes the *previous* milestone's upstream revision in `DEPS`
(`angle_revision`, etc.). That line replays **cleanly** onto the new milestone (no conflict — the
surrounding context matches), so nothing flags it — but the value now points at the OLD milestone's
dependency while the new milestone's gitlink/`gclient` expect the new one. After any rebase, diff
your `DEPS` against the target base (`git diff <NEW_BASE>:DEPS HEAD:DEPS`) and make the
fork-related vars **byte-identical to upstream** (the whole point of dropping the fork). Likewise
delete now-unused fork vars (e.g. `igalia_git`). For DEPS-managed submodules the *git gitlink* is
secondary to DEPS — resolve gitlink conflicts to the target milestone's upstream revision with
`git update-index --cacheinfo 160000,<sha>,<path>` (the fork object usually isn't even present
locally), and let `gclient sync` reconcile the working tree from DEPS.
