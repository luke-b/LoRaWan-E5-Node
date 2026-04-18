# Telemetry CI Updates - Deployment Guide

## 📋 Co se změnilo

Následující soubory byly vylepšeny pro lepší CI transparentnost a dokumentaci:

### Modifikované Soubory:
1. **Emulation/scripts/run_ci.sh**
   - ✅ Color-coded logging (GREEN/RED/YELLOW)
   - ✅ Toolchain verification step
   - ✅ Detailed build.log capture
   - ✅ ELF size validation
   - ✅ Artifact verification
   - ✅ Summary output

2. **.github/workflows/telemetry-emulation.yml**
   - ✅ Step-by-step logging
   - ✅ Artifact existence verification
   - ✅ GitHub step summary generation
   - ✅ Detailed error reporting

3. **docs/PROGRAMMERS_GUIDE.md**
   - ✅ BUILD_PROFILE documentation
   - ✅ Local CI setup instructions
   - ✅ 6-part troubleshooting guide
   - ✅ Docker and test scenario details

4. **README.md**
   - ✅ Docker CI instructions
   - ✅ GitHub Actions workflow info
   - ✅ Next steps for developers
   - ✅ Limitations and constraints

## 🚀 Fáze 3: Commit a Push (NYNÍ)

### Krok 1: Ověřit změny
```bash
cd /workspaces/LoRaWan-E5-Node
git status
```

Očekávaný výstup (4 modifikované soubory):
```
 M Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh
 M Projects/Applications/Telemetry/README.md
 M Projects/Applications/Telemetry/docs/PROGRAMMERS_GUIDE.md
 M .github/workflows/telemetry-emulation.yml
```

### Krok 2: Přidat soubory do staging
```bash
git add Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh
git add Projects/Applications/Telemetry/README.md
git add Projects/Applications/Telemetry/docs/PROGRAMMERS_GUIDE.md
git add .github/workflows/telemetry-emulation.yml
```

### Krok 3: Ověřit staged změny
```bash
git diff --cached --stat
```

### Krok 4: Commit se zprávu
```bash
git commit -m "enhance(telemetry): improve CI logging, documentation, and error handling

- Enhanced run_ci.sh with color-coded logging and detailed validation steps
- Updated .github/workflows/telemetry-emulation.yml with step summary generation
- Expanded docs/PROGRAMMERS_GUIDE.md with complete CI setup and troubleshooting guide
- Updated README.md with Docker CI and GitHub Actions instructions
- Added toolchain verification, artifact validation, and enhanced error reporting

This improves CI transparency and helps developers understand the emulation test flow."
```

### Krok 5: Push do GitHub
```bash
git push origin main
```

## ⏳ Co se stane po Push

GitHub Actions **automaticky** spustí workflow:

### Timeline:
1. **0s** - Trigger: push do main větve
2. **30s** - Checkout repo
3. **60-120s** - Docker build (stažení toolchain + Renode)
4. **120-180s** - Emulation firmware build
5. **180-240s** - Robot Framework testy v Renodě
6. **240s** - Artifact upload a summary generace

### Ověřit průběh:
1. Otevřít: https://github.com/Seeed-Studio/LoRaWan-E5-Node/actions
2. Vyhledat workflow: "Telemetry Emulation"
3. Kliknout na poslední run
4. Sledovat jednotlivé steps

## 📊 Fáze 4: Artifact Review (PO DOKONČENÍ CI)

### Stažení Artifacts:
1. Jdi na GitHub Actions workflow run
2. Klikni na "Artifacts" section
3. Stáhni: `telemetry-emulation-reports`

### Kontrola Výsledků:
```bash
# Rozbalit downloaded artifacts
unzip telemetry-emulation-reports.zip

# Prohlédnout build log
cat emulation-artifacts/build.log

# Prohlédnout test output
cat emulation-artifacts/uart.log

# Otevřít HTML report (v prohlížeči)
open emulation-artifacts/report.html  # macOS
xdg-open emulation-artifacts/report.html  # Linux
start emulation-artifacts/report.html  # Windows
```

### Co hledat v report.html:
```
Test Case: Should Boot Emulated Telemetry Harness
  Status: ✓ PASS
  Output: TELEMETRY EMULATION BOOT

Test Case: Should Validate Heartbeat And Alarm Payloads
  Status: ✓ PASS
  Assertions: ✓ 7/7

Test Case: Should Validate Counter Overflow Handling
  Status: ✓ PASS
  Assertions: ✓ 4/4
```

## ✅ Očekávané Výsledky

Pokud je vše správně:
- ✅ Docker image build: SUCCESS
- ✅ Compilation: SUCCESS (telemetry_emulation.elf ~100KB)
- ✅ All 4 robot tests: PASSED
- ✅ Artifact upload: SUCCESS
- ✅ GitHub step summary: Generated

## 🔧 Troubleshooting (Pokud CI selže)

### Pokud Docker build selže:
- Zkontroluj Dockerfile.emulation na syntax
- Ověř, že všechny URLs (toolchain, Renode) jsou dostupné

### Pokud compilation selže:
- Zkontroluj run_ci.sh - build.log by měl být v artifacts
- Ověř, že všechny #include cesty jsou správné

### Pokud Robot testy selžou:
- Zkontroluj uart.log pro specifické ASSERT FAIL řádky
- Ověř, že telemetry_emulation.elf byl vytvořen

### Pokud se workflow neactivuje:
- Zkontroluj, že jsi pushoval do `main` branch
- Ověř, že cesty v workflow match skutečné struktuře

## 📝 Příští Fáze (Po Úspěšném CI)

### Fáze 5: Hardware Testing (BUDOUCÍ)
```bash
# Kompiluj hardware firmware
make clean all  # Bez BUILD_PROFILE = hardware profil

# Copy to hardware via ST-Link
arm-none-eabi-objcopy -O binary telemetry.elf telemetry.bin

# Monitor UART output
screen /dev/ttyUSB0 115200
# nebo
minicom -D /dev/ttyUSB0 -b 115200

# Expected output:
# [boot message]
# [join status]
# [uplink count]
```

## 📚 Reference
- CI Script: [Emulation/scripts/run_ci.sh](./Projects/Applications/Telemetry/Emulation/scripts/run_ci.sh)
- Workflow: [.github/workflows/telemetry-emulation.yml](./.github/workflows/telemetry-emulation.yml)
- Guide: [docs/PROGRAMMERS_GUIDE.md](./Projects/Applications/Telemetry/docs/PROGRAMMERS_GUIDE.md)
- README: [Projects/Applications/Telemetry/README.md](./Projects/Applications/Telemetry/README.md)

---

**Všechny instrukce jsou připraveny. Pojď na krok 1 výše a spusť příkazy!** 🚀
