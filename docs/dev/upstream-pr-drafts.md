# Upstream PR Drafts

Prepared from fork `VultureZZ/lemonade` for submission to [lemonade-sdk/lemonade](https://github.com/lemonade-sdk/lemonade). Branches are pushed to origin for GitHub Compare review. **No PRs have been opened on upstream.**

Fork integration branch `main` retains all features plus local-only scripts. Upstream-facing branches exclude those scripts.

## Suggested merge order

1. `pr/request-logging`
2. `pr/download-sync`
3. `pr/model-manager`
4. `pr/default-model-fallback`

After each upstream merge, rebase remaining branches onto updated `upstream/main`.

---

## PR 1: Request logging

| Field | Value |
|-------|-------|
| Branch | `pr/request-logging` |
| Base | `lemonade-sdk/lemonade:main` |
| Commits | 16 (15 feature + 1 chore to drop local-only files) |
| Diff | 32 files, +4338 / −15 |
| Compare | https://github.com/lemonade-sdk/lemonade/compare/main...VultureZZ:pr/request-logging |

### Title

`feat(logging): add optional PostgreSQL request logging`

### Summary

- Adds optional PostgreSQL-backed HTTP request logging (`LEMONADE_REQUEST_LOG_*` env vars, CMake `LEMONADE_REQUEST_LOG` + libpq).
- Exposes review API endpoints: `/request-log/recent`, `/request-log/search`, `/request-log/stats`, `/request-log/clear` (quad-prefix registered).
- Adds desktop UI Request Log panel with payload viewer, plus docs, SQL schema, Docker quick-start, and integration tests.

### Config / breaking changes

- **Opt-in only** — disabled by default (`LEMONADE_REQUEST_LOG_ENABLED=false`).
- New env vars: `LEMONADE_REQUEST_LOG_ENABLED`, `LEMONADE_REQUEST_LOG_DATABASE_URL`, `LEMONADE_REQUEST_LOG_RETENTION_DAYS`, `LEMONADE_LOG_PROMPTS`.
- See [`docs/guide/configuration/request-log.md`](../guide/configuration/request-log.md) on the branch.

### Files excluded from this PR

These exist on fork `main` but were removed before upstream review:

- `scripts/deploy-ubuntu-local.sh`
- `examples/find-ollama-api-clients.sh`
- `data/secrets.conf`

### Test plan

- [ ] Build with libpq: `cmake --preset default && cmake --build --preset default`
- [ ] Unit tests: `./build/test_request_log_parser` (17 tests)
- [ ] Start PostgreSQL: `./examples/start-request-log-db.sh`
- [ ] Enable logging via env, restart `lemond`, send chat/completions requests
- [ ] Verify `/api/v1/request-log/recent` returns entries with redacted bodies
- [ ] Run `python test/server_request_log.py` (requires running server + PostgreSQL)
- [ ] Run `python test/server_endpoints.py` for clear endpoint coverage

### Local validation (2026-06-18)

- All four branches: `cmake --build --preset default` succeeded
- `test_request_log_parser`: 17 passed, 0 failed

---

## PR 2: Download synchronization

| Field | Value |
|-------|-------|
| Branch | `pr/download-sync` |
| Base | `lemonade-sdk/lemonade:main` |
| Commits | 1 |
| Diff | 5 files, +131 / −3 |
| Compare | https://github.com/lemonade-sdk/lemonade/compare/main...VultureZZ:pr/download-sync |

### Title

`feat(download): implement model download synchronization to prevent race conditions`

### Summary

- Adds per-model download mutex in `ModelManager` so concurrent `/pull`, `/load`, and auto-load paths cannot corrupt shared `.partial` files.
- Serializes pull handler work in `server.cpp` and adds shared HTTP client utilities.

### Config / breaking changes

- None — behavior change is internal concurrency safety only.

### Test plan

- [ ] Build: `cmake --build --preset default`
- [ ] Trigger concurrent `/pull` for the same model from two clients; verify single download completes without corruption
- [ ] Run `python test/server_endpoints.py` pull-related tests

### Local validation (2026-06-18)

- Build succeeded on this branch

---

## PR 3: Model manager settings and aliases

| Field | Value |
|-------|-------|
| Branch | `pr/model-manager` |
| Base | `lemonade-sdk/lemonade:main` |
| Commits | 3 |
| Diff | 15 files, +1558 / −443 |
| Compare | https://github.com/lemonade-sdk/lemonade/compare/main...VultureZZ:pr/model-manager |

### Title

`feat(models): add model settings modal, alias management, and downloaded-models UI`

### Summary

- Adds `ModelSettingsModal` and shared `ModelOptionsForm` for per-model recipe options and user-defined API aliases (persisted in `model_aliases.json`).
- Adds downloaded-models section with visibility filtering and delete actions in the model selector.
- Adds server endpoints for model settings (POST) and alias CRUD via `ModelManager`.

### Maintainer note

AGENTS.md states UI/frontend changes are handled by core maintainers. Consider opening an upstream Issue before submitting this PR.

### Config / breaking changes

- Model settings endpoint uses POST (replacing prior method — see `b6a6302b`).
- New persisted file: `model_aliases.json` in cache directory.

### Test plan

- [ ] Build: `cmake --build --preset default`
- [ ] Run `node test/app/app-regression/modelManagerActions.test.cjs`
- [ ] Run `python test/server_endpoints.py` model settings / alias tests
- [ ] Manual: open Model Manager, edit aliases, confirm API resolves alias names

### Local validation (2026-06-18)

- Build succeeded
- `modelManagerActions.test.cjs`: passed

---

## PR 4: Default model fallback

| Field | Value |
|-------|-------|
| Branch | `pr/default-model-fallback` |
| Base | `lemonade-sdk/lemonade:main` |
| Commits | 1 |
| Diff | 14 files, +225 / −37 |
| Compare | https://github.com/lemonade-sdk/lemonade/compare/main...VultureZZ:pr/default-model-fallback |

### Title

`feat(models): implement default model fallback in chat API`

### Summary

- Adds `default_model` to server runtime config (`config.json` / `POST /params`).
- When a chat request names an unknown model, OpenAI, Ollama, and Anthropic-compatible paths resolve to the configured default via `model_resolution.cpp`.
- Adds Settings panel and LLM Chat settings UI to select the default model.

### Config / breaking changes

- New runtime config key: `default_model` (empty string = disabled, prior behavior preserved).
- Default in `defaults.json`: empty.

### Test plan

- [ ] Build: `cmake --build --preset default`
- [ ] Set `default_model` via `/params` or UI
- [ ] Send Ollama `/api/chat` with unknown model; verify fallback (`test_012b_chat_falls_back_to_default_model` in `test/test_ollama.py`)
- [ ] Verify OpenAI `/v1/chat/completions` and Anthropic paths with unknown model
- [ ] Clear `default_model`; confirm unknown model returns error again

### Local validation (2026-06-18)

- Build succeeded
- Full `test/test_ollama.py` not run (requires inference backend and model pull)

---

## Publishing checklist (when ready)

Do **not** run these until upstream maintainers are ready to review:

```bash
# Example — one PR at a time, in merge order
gh pr create --repo lemonade-sdk/lemonade \
  --base main \
  --head VultureZZ:pr/request-logging \
  --title "feat(logging): add optional PostgreSQL request logging" \
  --body-file /path/to/pr1-body.md \
  --draft
```

1. Open upstream Issues for large features (especially PR 3 UI work)
2. Rebase each branch onto latest `upstream/main` if prior PRs merged
3. Create draft PRs with bodies from this document
4. Link Issues and paste test plan checklists
