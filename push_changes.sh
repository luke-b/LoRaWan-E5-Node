#!/bin/bash
# Git Push Script for Telemetry CI Updates
# Usage: bash push_changes.sh (run from repository root)

set -euo pipefail

echo "==========================================="
echo "Telemetry CI Updates - Git Push Script"
echo "==========================================="
echo ""

# Verify we're in the right repo
if [ ! -f ".git/config" ]; then
    echo "❌ Error: Not in a git repository root directory"
    echo "   Please run this script from the repository root"
    exit 1
fi

# Verify git is available
if ! command -v git &> /dev/null; then
    echo "❌ Error: git is not installed or not in PATH"
    exit 1
fi

echo "[1/5] Verifying repository..."
REPO_NAME=$(git remote get-url origin | sed 's/.*\///' | sed 's/\.git$//')
echo "     Repository: $REPO_NAME"
BRANCH=$(git rev-parse --abbrev-ref HEAD)
echo "     Current branch: $BRANCH"
echo ""

echo "[2/5] Checking git status..."
git status --short
echo ""

echo "[3/5] Adding modified files..."
git add Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh
git add Projects/Applications/Telemetry/README.md
git add Projects/Applications/Telemetry/docs/PROGRAMMERS_GUIDE.md
git add .github/workflows/telemetry-emulation.yml
echo "     ✓ Files staged"
echo ""

echo "[4/5] Reviewing changes..."
git diff --cached --stat
echo ""

echo "[5/5] Creating commit and pushing..."
git commit -m "enhance(telemetry): improve CI logging, documentation, and error handling

- Enhanced run_ci.sh with color-coded logging and detailed validation steps
- Updated .github/workflows/telemetry-emulation.yml with step summary generation
- Expanded docs/PROGRAMMERS_GUIDE.md with complete CI setup and troubleshooting guide
- Updated README.md with Docker CI and GitHub Actions instructions
- Added toolchain verification, artifact validation, and enhanced error reporting

This improves CI transparency and helps developers understand the emulation test flow."

echo ""
echo "Pushing to GitHub..."
git push origin "$BRANCH"

echo ""
echo "==========================================="
echo "✅ SUCCESS! Changes pushed to GitHub"
echo "==========================================="
echo ""
echo "GitHub Actions will automatically start the workflow:"
echo "  URL: https://github.com/Seeed-Studio/LoRaWan-E5-Node/actions"
echo ""
echo "Expected workflow duration: 5-10 minutes"
echo "  - Docker image build: 60-120s"
echo "  - Emulation compilation: 30-60s"
echo "  - Robot tests: 30-60s"
echo "  - Artifact upload: 10-20s"
echo ""
