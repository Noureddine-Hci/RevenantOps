# RevenantOps - Codex Project Memory

Read these files as project context before working:
- @README.md
- @primer.md
- @ROADMAP.md
- @DEVLOG.md

Optional local tooling reference:
- @launch.json

## Project
- **RevenantOps**: third-person shooter / Mercenaries mode in Unreal Engine 5.7, C++
- Developers: **Noureddine Houichi** and **Jilani**
- User-facing communication language: **French**
- Code, rules, and technical prompts may be in **English** for precision

## Session Start Rule
- At the start of every session, identify the active developer.
- If not explicitly provided, ask: `Tu es Noureddine ou Jilani ?`
- Inspect the current branch, modified files, and current project state before changing anything.

## Source Of Truth
- `README.md` = short project overview
- `primer.md` = current state, active branch, immediate priorities, known issues
- `ROADMAP.md` = long-term backlog and feature planning
- `DEVLOG.md` = historical journal, context, and past implementation notes
- If documentation and project state disagree, inspect the code/assets and report the mismatch.

## Core Unreal Rules
- Use Unreal Engine 5.7 APIs only. Avoid deprecated APIs unless explicitly justified.
- Use Enhanced Input only. Never reintroduce legacy input.
- Prefer C++ for core gameplay systems.
- Prefer Blueprints for lightweight assembly, tuning, and presentation.
- Prefer Python for editor automation and repetitive editor tasks.
- Avoid plugin sprawl and hidden Blueprint logic.

## C++ Conventions
- `SetCrouchedHalfHeight()` instead of writing deprecated crouch properties directly
- Cast `AController` to `APlayerController` before calling `AddPitchInput` / `AddYawInput`
- Abstract gameplay classes require Blueprint children where the project expects them
- Keep Unreal naming conventions: `U/A/F/S/E` prefixes and CamelCase
- Avoid raw `new` unless there is a strong reason and ownership is explicit
- Use `UPROPERTY` / `UFUNCTION` appropriately for GC, reflection, and Blueprint integration

## Required Build Modules
- Engine
- InputCore
- EnhancedInput
- AIModule
- NavigationSystem
- StateTreeModule
- UMG
- Slate
- EngineCameras when camera shake classes require it

## Debugging Method
- Work in short cycles: inspect, diagnose, propose, change, test, report.
- Never guess when you can inspect code, logs, assets, editor state, or build output.
- Prefer small targeted fixes over broad refactors.
- If a bug persists after 2 attempts, stop patching and switch to strict diagnosis mode.
- Change one main variable at a time when debugging.
- Do not edit unrelated systems.

## Animation And Posture Bugs
- Reproduce the issue first.
- Isolate the faulty layer before editing:
  - skeletal mesh / skeleton
  - animation blueprint
  - state machine / blendspace
  - retargeting
  - montages / layered blends
  - aim offset
  - IK / Control Rig / post-process animation
  - character blueprint
  - C++ gameplay code
- For posture or animation bugs, test with a minimal setup before touching gameplay logic.
- If needed, create the smallest possible reproduction.

## MCP Rules
- Never change level through the Unreal MCP TCP `Open` command.
- Never send MCP commands while PIE is actively running if the connection is known to block.
- Configure assets, actors, and data in Editor mode or Simulate mode before asking for PIE validation.
- Ask the user to open levels manually when needed.

## Build And Validation Loop
For any meaningful C++ change:
1. Modify `.h` / `.cpp`
2. Compile with the correct Unreal build flow
3. If compile fails, read the exact error and fix only the real cause
4. Test in PIE, automation tests, or the smallest useful validation path
5. If test fails, inspect logs and fix the logic
6. Report what was actually verified

Do not declare a task complete without evidence that it compiles and was validated appropriately.

## Completion Checklist
- Code compiles without errors
- Relevant test or PIE validation completed
- No critical warnings introduced
- Unreal conventions respected
- No unnecessary duplication
- No avoidable ownership or GC issues

## Collaboration Rules
- `main` must stay stable, buildable, and PIE-valid
- Never work directly on `main`
- Branch naming:
  - `noureddine/phase-XX`
  - `jilani/phase-XX`
- Commit format:
  - `[N] feat(scope): description`
  - `[J] feat(scope): description`
- UE `.uasset` files are binary and non-mergeable: only one developer edits a given asset at a time
- Always announce ownership before editing a shared map, animation blueprint, or major Blueprint asset

## Sensitive Assets
Only one developer at a time should edit:
- `Lvl_ThirdPerson.umap`
- `ABP_Mercenaire`
- major gameplay Blueprints
- shared UI Blueprints

## Safe Reporting Format
For each substantial task, provide:
1. Problem summary
2. Likely cause
3. Verification performed
4. Changes made
5. Result
6. Remaining risk

## Persistent Memory Discipline
- Update `primer.md` at the start and end of each session
- Update `AGENTS.md` only when permanent project rules or durable workflow knowledge changes
- Keep temporary session notes in `primer.md` or `DEVLOG.md`, not in `AGENTS.md`

## Known Unreal Notes

### Editor Screenshot Pattern
```python
unreal.AutomationLibrary.take_high_res_screenshot(1920, 1080, "nom_fichier")
unreal.EditorLevelLibrary.set_level_viewport_camera_info(
    unreal.Vector(x, y, z),
    unreal.Rotator(pitch=pitch, yaw=yaw, roll=0)
)
```

### Camera Rotator Warning
- Do not rely on positional constructor order for `Rotator`; use keyword arguments for clarity.

### Level Design Height Formula
- Floor top = `Z = -37.5`
- Object on floor = `center_z = -37.5 + scale_z * 50`
- Raised object by height `H` = `center_z = -37.5 + H + scale_z * 50`

### DataTable Blueprint CDO Pattern
```python
bp = unreal.load_asset(path)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
cdo = unreal.get_default_object(bp.generated_class())
handle = unreal.DataTableRowHandle(data_table=dt, row_name='RowName')
with unreal.ScopedEditorTransaction('Set prop'):
    bp.modify()
    cdo.modify()
    cdo.set_editor_property('prop_name', handle)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
unreal.EditorAssetLibrary.save_asset(path)
```

## Current Focus
- Keep using `primer.md` for the active phase, current blockers, and next steps.
- Treat `ROADMAP.md` as backlog, not as immediate implementation order.
