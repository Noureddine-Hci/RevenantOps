---
phase: 10
slug: editor-setup-playtest
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-03-23
---

# Phase 10 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | MCP Python + PIE manual |
| **Config file** | none — editor-only phase |
| **Quick run command** | `MCP: hasattr(unreal, 'ClassName')` checks |
| **Full suite command** | PIE Play — manual 5-min match |
| **Estimated runtime** | ~10 minutes (full playtest) |

---

## Sampling Rate

- **After every task commit:** MCP Python verification (actor exists, properties set)
- **After every plan wave:** Visual check in editor viewport
- **Before `/gsd:verify-work`:** Full PIE playtest (title→loadout→match→gameover→leaderboard)
- **Max feedback latency:** 30 seconds (MCP checks), 10 min (PIE)

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Test Type | Automated Command | Status |
|---------|------|------|-------------|-----------|-------------------|--------|
| 10-01-01 | 01 | 1 | AREN-01 | MCP | spawn point actors exist in level | ⬜ pending |
| 10-01-02 | 01 | 1 | AREN-03 | MCP | pickup actors exist in level | ⬜ pending |
| 10-01-03 | 01 | 1 | AREN-01 | MCP | NavMesh built (NavMeshBoundsVolume exists) | ⬜ pending |
| 10-01-04 | 01 | 1 | AREN-04 | MCP | Directional/Point lights in level | ⬜ pending |
| 10-02-01 | 02 | 2 | UI-01 | MCP | WBP widgets have child widgets | ⬜ pending |
| 10-02-02 | 02 | 2 | UI-02 | MCP | PlayerController widget class properties set | ⬜ pending |
| 10-03-01 | 03 | 3 | FX-01 | MCP | Weapon BPs have FireSound set | ⬜ pending |
| 10-04-01 | 04 | 4 | all | manual | Full PIE playtest 5min | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

*Existing infrastructure covers all phase requirements. No test framework needed — this is an editor-only phase verified via MCP Python and manual PIE.*

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Zombies navigate via NavMesh | AREN-01 | Requires PIE runtime | Launch PIE, verify zombies path to player |
| Widget buttons respond to clicks | UI-01/03 | Requires mouse input in PIE | Click Play/Quit/Replay buttons |
| Full flow title→leaderboard | UI-04 | End-to-end requires PIE | Play complete 5-min match |
| Audio plays on fire/reload/death | FX-01/02/03 | Requires audio playback | Listen during PIE |
| Camera OTS feels correct | CAM-01 | Subjective feel | Observe camera during gameplay |

---

## Validation Sign-Off

- [ ] All tasks have MCP verify or manual PIE test
- [ ] Sampling continuity: MCP check after each editor task
- [ ] No watch-mode flags
- [ ] Feedback latency < 30s (MCP), < 10min (PIE)
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
