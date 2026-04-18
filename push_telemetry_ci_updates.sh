#!/usr/bin/env bash

# Script to commit and push Telemetry CI improvements to GitHub
# Run this from repository root: bash push_telemetry_ci_updates.sh

set -euo pipefail

echo "================================"
echo "Telemetry CI Updates Push Script"
echo "================================"
echo ""

# Change to repo root
cd "$(git rev-parse --show-toplevel)"

echo "[1] Checking git status..."
git status --short || true
echo ""

echo "[2] Adding modified files..."
git add Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh
git add Projects/Applications/Telemetry/README.md
git add Projects/Applications/Telemetry/docs/PROGRAMMERS_GUIDE.md
git add .github/workflows/telemetry-emulation.yml

echo "[3] Reviewed changes:"
git diff --cached --stat

echo ""
echo "[4] Committing changes..."
git commit -m "enhance(telemetry): improve CI logging, documentation, and error handling

- Enhanced run_ci.sh with color-coded logging and detailed validation steps
- Updated .github/workflows/telemetry-emulation.yml with step summary generation
- Expanded docs/PROGRAMMERS_GUIDE.md with complete CI setup and troubleshooting guide
- Updated README.md with Docker CI and GitHub Actions instructions
- Added toolchain verification, artifact validation, and enhanced error reporting

This improves CI transparency and helps developers understand the emulation test flow."

echo ""
echo "[5] Pushing to remote..."
git push origin HEAD:main

echo ""
echo "✅ Successfully pushed Telemetry CI improvements!"
echo ""
echo "GitHub Actions will now automatically:"
echo "  1. Build Docker image"
echo "  2. Compile emulation firmware"
echo "  3. Run Robot Framework tests"
echo "  4. Generate test reports"
echo "  5. Upload artifacts"
echo ""
echo "Monitor progress at:"
echo "  https://github.com/Seeed-Studio/LoRaWan-E5-Node/actions"
echo ""
