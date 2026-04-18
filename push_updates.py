#!/usr/bin/env python3
"""
Git Push Script for Telemetry CI Updates
Handles commit and push without terminal access issues
"""

import subprocess
import sys
from pathlib import Path

def run_git_command(repo_path, *args):
    """Run a git command in the specified repository"""
    cmd = ['git', '-C', str(repo_path)] + list(args)
    print(f"Running: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        if result.stdout:
            print("STDOUT:", result.stdout)
        if result.stderr:
            print("STDERR:", result.stderr)
        if result.returncode != 0:
            print(f"❌ Command failed with return code {result.returncode}")
            return False
        print("✅ Command succeeded")
        return True
    except subprocess.TimeoutExpired:
        print(f"❌ Command timed out")
        return False
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def main():
    repo_path = Path("/workspaces/LoRaWan-E5-Node")
    
    if not repo_path.exists():
        print(f"❌ Repository not found: {repo_path}")
        sys.exit(1)
    
    print("=" * 60)
    print("Telemetry CI Updates - Git Automation")
    print("=" * 60)
    print()
    
    # Step 1: Check status
    print("[1] Checking git status...")
    run_git_command(repo_path, "status", "--porcelain")
    print()
    
    # Step 2: Add files
    print("[2] Adding modified files...")
    files_to_add = [
        "Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh",
        "Projects/Applications/Telemetry/README.md",
        "Projects/Applications/Telemetry/docs/PROGRAMMERS_GUIDE.md",
        ".github/workflows/telemetry-emulation.yml",
    ]
    
    for file in files_to_add:
        if not run_git_command(repo_path, "add", file):
            print(f"⚠️  Failed to add {file}")
    print()
    
    # Step 3: Show diff
    print("[3] Showing staged changes...")
    run_git_command(repo_path, "diff", "--cached", "--stat")
    print()
    
    # Step 4: Commit
    print("[4] Creating commit...")
    commit_message = """enhance(telemetry): improve CI logging, documentation, and error handling

- Enhanced run_ci.sh with color-coded logging and detailed validation steps
- Updated .github/workflows/telemetry-emulation.yml with step summary generation
- Expanded docs/PROGRAMMERS_GUIDE.md with complete CI setup and troubleshooting guide
- Updated README.md with Docker CI and GitHub Actions instructions
- Added toolchain verification, artifact validation, and enhanced error reporting

This improves CI transparency and helps developers understand the emulation test flow."""
    
    if not run_git_command(repo_path, "commit", "-m", commit_message):
        print("⚠️  Commit failed - may already be committed")
    print()
    
    # Step 5: Push
    print("[5] Pushing to remote...")
    if run_git_command(repo_path, "push", "origin", "main"):
        print()
        print("=" * 60)
        print("✅ Successfully pushed Telemetry CI improvements!")
        print("=" * 60)
        print()
        print("GitHub Actions will now automatically run.")
        print("Monitor progress at:")
        print("  https://github.com/Seeed-Studio/LoRaWan-E5-Node/actions")
        print()
        sys.exit(0)
    else:
        print("❌ Push failed - check network and authentication")
        sys.exit(1)

if __name__ == "__main__":
    main()
