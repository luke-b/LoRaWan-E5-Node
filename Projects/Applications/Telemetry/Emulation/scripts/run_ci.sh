#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
ARTIFACTS_DIR="${PROJECT_DIR}/emulation-artifacts"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Logging function
log() {
  echo -e "${GREEN}[CI]${NC} $*"
}

error() {
  echo -e "${RED}[ERROR]${NC} $*" >&2
}

warn() {
  echo -e "${YELLOW}[WARN]${NC} $*"
}

# Create artifacts directory
mkdir -p "${ARTIFACTS_DIR}"
log "Created artifacts directory: ${ARTIFACTS_DIR}"

# Navigate to project root
cd "${PROJECT_DIR}"
log "Working directory: $(pwd)"

# Step 1: Verify toolchain
log "Step 1: Verifying toolchain..."
if ! command -v arm-none-eabi-gcc &> /dev/null; then
  error "arm-none-eabi-gcc not found in PATH"
  exit 1
fi
log "  ✓ arm-none-eabi-gcc available"

if ! command -v renode-test &> /dev/null; then
  error "renode-test not found in PATH"
  exit 1
fi
log "  ✓ renode-test available"

# Step 2: Clean and build
log "Step 2: Building emulation firmware (BUILD_PROFILE=emulation)..."
if ! make BUILD_PROFILE=emulation clean all 2>&1 | tee "${ARTIFACTS_DIR}/build.log"; then
  error "Compilation failed. See ${ARTIFACTS_DIR}/build.log for details."
  exit 1
fi
log "  ✓ Build completed successfully"

# Verify ELF was created
if [[ ! -f "${PROJECT_DIR}/telemetry_emulation.elf" ]]; then
  error "telemetry_emulation.elf was not generated!"
  exit 1
fi
log "  ✓ ELF generated: $(wc -c < ${PROJECT_DIR}/telemetry_emulation.elf) bytes"

# Step 3: Run Renode tests
log "Step 3: Running Renode emulation tests..."
if ! renode-test Emulation/tests/integration_tests.robot \
  > >(tee "${ARTIFACTS_DIR}/uart.log") \
  2> >(tee -a "${ARTIFACTS_DIR}/uart.log" >&2); then
  error "Renode tests failed. See ${ARTIFACTS_DIR}/uart.log for details."
  exit 1
fi
log "  ✓ All emulation tests passed"

# Step 4: Collect reports
log "Step 4: Collecting test reports..."
report_count=0
for file in report.html log.html; do
  if [[ -f "${PROJECT_DIR}/${file}" ]]; then
    cp "${PROJECT_DIR}/${file}" "${ARTIFACTS_DIR}/${file}"
    log "  ✓ Copied ${file}"
    report_count=$((report_count + 1))
  else
    warn "  Report not found: ${file}"
  fi
done

# Step 5: Copy ELF to artifacts
cp "${PROJECT_DIR}/telemetry_emulation.elf" "${ARTIFACTS_DIR}/"
log "  ✓ Copied telemetry_emulation.elf"

# Final summary
log ""
log "========== CI SUMMARY =========="
log "Build:     ✓ PASSED"
log "Tests:     ✓ PASSED"
log "Reports:   ✓ ${report_count} file(s)"
log "Artifacts: ${ARTIFACTS_DIR}"
log "=============================="
log "CI completed successfully!"