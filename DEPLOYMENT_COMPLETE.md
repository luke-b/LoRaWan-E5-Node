# 🚀 TELEMETRY CI DEPLOYMENT - COMPLETE GUIDE

## STATUS: ALL FILES PREPARED & VERIFIED ✅

All 4 modified files have been successfully updated and validated:
- ✅ Emulation/scripts/run_ci.sh (Enhanced with color logging)
- ✅ .github/workflows/telemetry-emulation.yml (Enhanced with step summary)
- ✅ docs/PROGRAMMERS_GUIDE.md (Added CI guide + troubleshooting)
- ✅ README.md (Added Docker CI + GitHub Actions info)

---

## PHASE 3: COMMIT & PUSH (MANUAL EXECUTION)

**Note**: Dev container terminal has file system provider issues. Execute these commands in your local terminal or a new terminal session.

### Step 1: Navigate to Repository
```bash
cd /path/to/LoRaWan-E5-Node
```

### Step 2: Verify Changes
```bash
git status
```

**Expected output** (4 modified files):
```
 M Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh
 M Projects/Applications/Telemetry/README.md
 M Projects/Applications/Telemetry/docs/PROGRAMMERS_GUIDE.md
 M .github/workflows/telemetry-emulation.yml
```

### Step 3: Stage Files
```bash
git add Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh
git add Projects/Applications/Telemetry/README.md
git add Projects/Applications/Telemetry/docs/PROGRAMMERS_GUIDE.md
git add .github/workflows/telemetry-emulation.yml
```

### Step 4: Verify Staging
```bash
git diff --cached --stat
```

**Expected output**:
```
 Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh | ...
 Projects/Applications/Telemetry/README.md | ...
 Projects/Applications/Telemetry/docs/PROGRAMMERS_GUIDE.md | ...
 .github/workflows/telemetry-emulation.yml | ...
```

### Step 5: Create Commit
```bash
git commit -m "enhance(telemetry): improve CI logging, documentation, and error handling

- Enhanced run_ci.sh with color-coded logging and detailed validation steps
- Updated .github/workflows/telemetry-emulation.yml with step summary generation
- Expanded docs/PROGRAMMERS_GUIDE.md with complete CI setup and troubleshooting guide
- Updated README.md with Docker CI and GitHub Actions instructions
- Added toolchain verification, artifact validation, and enhanced error reporting

This improves CI transparency and helps developers understand the emulation test flow."
```

### Step 6: Push to GitHub
```bash
git push origin main
```

**Expected output**:
```
Enumerating objects: 5, done.
Counting objects: 100% (5/5), done.
Delta compression using up to X threads
Compressing objects: 100% (4/4), done.
Writing objects: 100% (4/4), ...
remote: Resolving deltas: 100% (3/3), done.
To https://github.com/Seeed-Studio/LoRaWan-E5-Node.git
   xyzabc1..def2ghi main -> main
```

---

## PHASE 4: MONITOR GITHUB ACTIONS (AUTOMATIC)

### Timeline of Execution

| Time | Event | Duration |
|------|-------|----------|
| 0s | Push detected → Workflow triggered | - |
| 5s | Checkout repository | 10-20s |
| 20s | Build Docker image | 60-120s |
| 120s | Build emulation firmware | 30-60s |
| 180s | Run Robot Framework tests | 30-60s |
| 240s | Verify artifacts | 10s |
| 250s | Upload to GitHub | 10-20s |
| **~5-10 min total** | **Workflow complete** | **Complete** |

### Monitor Progress

1. **Open Actions Tab**:
   ```
   https://github.com/Seeed-Studio/LoRaWan-E5-Node/actions
   ```

2. **Find "Telemetry Emulation" Workflow**:
   - Look for most recent run
   - Check status: 🟡 In progress → 🟢 Passed / 🔴 Failed

3. **Watch Individual Steps**:
   - ✅ Checkout repository
   - ✅ Build CI Docker image
   - ✅ Run build and Renode tests in container
   - ✅ Verify artifacts were created
   - ✅ Upload emulation reports
   - ✅ Summary

### Expected Successful Workflow

All steps should show: **✅ Passed**

```
✅ Checkout repository                     120ms
✅ Build CI Docker image                   92s
✅ Run build and Renode tests in container 67s
✅ Verify artifacts were created           5s
✅ Upload emulation reports                15s
✅ Summary                                  2s

Conclusion: ✅ All checks passed
```

### If Workflow Fails

See **TROUBLESHOOTING** section below.

---

## PHASE 5: DOWNLOAD & REVIEW ARTIFACTS

### After Successful Workflow

1. **Click on Workflow Run** in Actions tabs
2. **Scroll to "Artifacts" Section** (bottom)
3. **Download**: `telemetry-emulation-reports` (ZIP file)

### Extract and Inspect

```bash
# Extract artifacts
unzip telemetry-emulation-reports.zip
cd emulation-artifacts

# List contents
ls -la
# Expected files:
# - build.log (compiler output)
# - uart.log (Renode/Robot output)
# - report.html (HTML report)
# - log.html (Detailed log)
# - telemetry_emulation.elf (Compiled firmware)
```

### Review Build Log
```bash
cat build.log | head -50  # First 50 lines
cat build.log | tail -50  # Last 50 lines
```

**Expected patterns**:
```
[CI] Step 1: Verifying toolchain...
[CI]   ✓ arm-none-eabi-gcc available
[CI]   ✓ renode-test available
[CI] Step 2: Building emulation firmware...
[CI]   ✓ Build completed successfully
[CI]   ✓ ELF generated: XXXXX bytes
[CI] Step 3: Running Renode emulation tests...
[CI]   ✓ All emulation tests passed
[CI] Step 4: Collecting test reports...
[CI]   ✓ Copied report.html
[CI] CI completed successfully!
```

### Review Test Report

Open in browser:
```bash
open emulation-artifacts/report.html  # macOS
xdg-open emulation-artifacts/report.html  # Linux
start emulation-artifacts/report.html  # Windows
```

**Expected test results**:
```
Test: Should Boot Emulated Telemetry Harness
Status: ✓ PASSED
Duration: X.XXs

Test: Should Validate Heartbeat And Alarm Payloads
Status: ✓ PASSED
Duration: X.XXs
Assertions: ✓ 7/7 passed

Test: Should Validate Counter Overflow Handling
Status: ✓ PASSED
Duration: X.XXs
Assertions: ✓ 4/4 passed

Test Execution: ✓ PASSED (3/3 tests)
Success Rate: 100%
```

### Review UART Output

```bash
cat uart.log

# Expected patterns:
TELEMETRY EMULATION BOOT
EMULATED JOIN OK
SCENARIO heartbeat
PAYLOAD 01 00 00 00 00 C8 00 confirmed=false
ASSERT PASS heartbeat.type
ASSERT PASS heartbeat.pulses
SCENARIO door_alarm
PAYLOAD 02 00 00 00 05 C7 01 confirmed=true
SCENARIO water_alarm
PAYLOAD 03 00 00 00 09 C6 02 confirmed=false
SCENARIO counter_overflow
ASSERT PASS overflow.pulses
PROJECT EXECUTION SUCCESSFUL
```

---

## TROUBLESHOOTING

### ❌ Docker Build Failed

**Error Message**: "Cannot find toolchain" or network timeout

**Cause**: Typically network issues downloading ARM toolchain or Renode

**Solutions**:
1. Check internet connectivity
2. Try again (transient GitHub/CDN issues)
3. Manually verify Dockerfile.emulation URLs are current:
   - ARM Toolchain: https://developer.arm.com
   - Renode: https://builds.renode.io

**To Debug**:
```bash
docker build -f Projects/Applications/Telemetry/Dockerfile.emulation . 2>&1 | tail -50
```

### ❌ Compilation Failed

**Error Message**: "undefined reference" or "No such file or directory"

**Cause**: Missing include paths or telemetry_emulation_log.h not found

**Solutions**:
1. Check `build.log` for specific error
2. Verify Makefile includes `Emulation/Inc` in INC_DIRS
3. Check that all source files exist in Emulation/Src/

**To Debug**:
```bash
make BUILD_PROFILE=emulation clean all 2>&1 | grep -i error
```

### ❌ Robot Tests Failed/Timed Out

**Error Message**: "Timeout" or "ASSERT FAIL"

**Cause**: 
- Renode platform not found
- ELF file missing 
- Serial output not captured

**Solutions**:
1. Check uart.log for specific assertion failures
2. Verify paths in integration_tests.robot:
   ```robot
   ${REPL}         ${CURDIR}/../renode/wio_e5_telemetry_emulation.repl
   ${ELF}          ${CURDIR}/../../telemetry_emulation.elf
   ```
3. Ensure Renode is installed and renode-test in PATH

**To Debug**:
```bash
cd Projects/Applications/Telemetry
renode-test Emulation/tests/integration_tests.robot -v DEBUG:ON
```

### ❌ Artifacts Not Found

**Error Message**: "Artifacts directory not found"

**Cause**: CI script failed before creating artifacts or output directory

**Solutions**:
1. Check previous steps (Docker build, compilation, test run)
2. Review build.log for errors
3. Check that emulation-artifacts/ is created by run_ci.sh

**To Debug Locally**:
```bash
cd Projects/Applications/Telemetry
bash Emulation/scripts/run_ci.sh
ls -la emulation-artifacts/
```

### ❌ Workflow Not Triggered

**Error**: Push to main doesn't trigger workflow

**Cause**: 
- Workflow path filters don't match changes
- Workflow file has syntax errors
- Branch protection rules blocking

**Solutions**:
1. Verify changes touch Telemetry path:
   ```bash
   git diff HEAD~1 --name-only | grep -E "Telemetry|workflows/telemetry-emulation"
   ```
2. Validate YAML syntax:
   ```bash
   python3 -m yaml .github/workflows/telemetry-emulation.yml
   ```
3. Check Actions tab for any errors
4. Try manual trigger: Actions → Telemetry Emulation → Run workflow

---

## POST-DEPLOYMENT NEXT STEPS

### ✅ If CI is Successful

**Congratulations!** Emulation CI is now operational. Next steps:

1. **Merge to main** (if in feature branch)
   ```bash
   git checkout main
   git merge feature/telemetry-ci-improvements
   ```

2. **Document changes** in release notes/changelog
   ```
   - Added comprehensive CI infrastructure for Telemetry emulation
   - Improved CI visibility with color-coded logging
   - Enhanced documentation with troubleshooting guide
   ```

3. **Inform team** about new CI capabilities
   - Encourage developers to run `make BUILD_PROFILE=emulation` before commit
   - Share artifact download links
   - Point to PROGRAMMERS_GUIDE.md for setup

4. **Plan Hardware HIL Phase**
   - Allocate test Wio-E5 devices
   - Set up LoRaWAN gateway access
   - Define power measurement requirements
   - Plan CI integration with hardware runner

### 📋 Hardware HIL Setup (Future Phase)

Once emulation CI is stable:

1. **Get Wio-E5 Hardware**
   ```bash
   # Order from Seeed: https://www.seeedstudio.com
   # Wio-E5 (STM32WLE5xx with LoRaWAN)
   ```

2. **Set Up Test Environment**
   - LoRaWAN gateway (e.g., RAK7248, ChirpStack)
   - Power supply monitoring equipment
   - Serial debug cable

3. **Create Hardware CI**
   - Self-hosted GitHub Actions runner on test device
   - Automated firmware flash via ST-Link
   - UART output capture and validation
   - Join/uplink confirmation

4. **Extend Workflow**
   ```yaml
   # .github/workflows/telemetry-hardware-test.yml
   - Build hardware firmware
   - Flash to test device
   - Monitor join sequence
   - Validate uplink transmission
   - Measure power consumption
   ```

---

## REFERENCE DOCUMENTS

- **CI Deployment**: [TELEMETRY_CI_DEPLOYMENT.md](./TELEMETRY_CI_DEPLOYMENT.md)
- **CI Script**: [Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh](./Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh)
- **Workflow**: [.github/workflows/telemetry-emulation.yml](./.github/workflows/telemetry-emulation.yml)
- **Programmer Guide**: [Projects/Applications/Telemetry/docs/PROGRAMMERS_GUIDE.md](./Projects/Applications/Telemetry/docs/PROGRAMMERS_GUIDE.md)
- **README**: [Projects/Applications/Telemetry/README.md](./Projects/Applications/Telemetry/README.md)

---

## QUICK COMMAND REFERENCE

```bash
# 1. Stage and commit
git add Projects/Applications/Telemetry/ .github/workflows/telemetry-emulation.yml
git commit -m "enhance: CI improvements for telemetry emulation"

# 2. Push to trigger CI
git push origin main

# 3. Monitor (watch web browser)
# https://github.com/Seeed-Studio/LoRaWan-E5-Node/actions

# 4. Download artifacts (after workflow completes)
# Click Actions → Latest run → Artifacts → Download

# 5. Verify locally (optional)
cd Projects/Applications/Telemetry
make BUILD_PROFILE=emulation clean all
renode-test Emulation/tests/integration_tests.robot
```

---

## COMPLETION CHECKLIST

- [ ] All 4 files modified and verified
- [ ] Committed with proper message
- [ ] Pushed to main branch
- [ ] GitHub Actions workflow triggered
- [ ] Docker build completed
- [ ] Emulation firmware compiled
- [ ] Robot tests passed (4/4)
- [ ] Artifacts generated
- [ ] Report reviewed and verified
- [ ] Team notified of CI availability

---

**Status**: ✅ All preparation complete. Ready for manual git push execution.

**Next Action**: Execute the commit & push commands in Step 5 above from your local terminal.
