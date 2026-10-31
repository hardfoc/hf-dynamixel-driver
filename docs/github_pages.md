---
layout: default
title: "GitHub Pages"
description: "Publish Jekyll + Doxygen with hf-general-ci-tools"
parent: "Documentation"
nav_order: 10
permalink: /docs/github_pages/
---

# Publishing GitHub Pages

Documentation is built by `.github/workflows/ci-docs-publish.yml`, which
calls
[`hardfoc/hf-general-ci-tools` `ru-docs-publish.yml`](https://github.com/hardfoc/hf-general-ci-tools).
That reusable workflow:

1. Checks out this repo **recursively** (DynamixelSDK + ESP32 scripts).
2. Runs Doxygen with `_config/Doxyfile` (API HTML under `docs/html`).
3. Builds Jekyll from `_config/_config.yml` (just-the-docs remote theme).
4. Optionally link-checks and markdown-lints.
5. Pushes the combined site to the `gh-pages` branch when the event is a
   push to `main` / `release/*` or a `v*` tag.

## One-time repository settings

In **GitHub → Settings → Pages**:

1. Source: **Deploy from a branch**.
2. Branch: `gh-pages` / `/` (root).
3. Wait for the first green `📚 Docs Publish CI` run on `main`.

The published URL is
`https://hardfoc.github.io/hf-dynamixel-driver/`
(`baseurl: /hf-dynamixel-driver`). Doxygen is linked as
`https://hardfoc.github.io/hf-dynamixel-driver/html/`.

Pull requests run the same workflow with `deploy_pages: false` so the
theme, Doxygen, and link check fail in CI before merge.

## Local preview

```bash
# API
doxygen _config/Doxyfile

# Site (needs Bundler + just-the-docs remote theme)
bundle exec jekyll serve --config _config/_config.yml
```

## Companion workflows

| Workflow | Purpose |
|----------|---------|
| `ci-docs-publish.yml` | Jekyll + Doxygen + optional deploy |
| `ci-docs-linkcheck.yml` | lychee via `_config/lychee.toml` |
| `ci-markdown-lint.yml` | `_config/.markdownlint.json` |
| `ci-yaml-lint.yml` | `_config/.yamllint` |
| `host-tests.yml` | CMake/CTest + install-prefix smoke |
| `esp32-examples-build-ci.yml` | `generate_matrix.py` + `hf-espidf-ci-tools` |
| `release.yml` | GitHub release on `v*` tags |

`_config/` also holds `.clang-format` / `.clang-tidy` for `ci-cpp-lint.yml`.
SDK sources under `external/` are excluded from that lint.
