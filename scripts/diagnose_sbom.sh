#! /usr/bin/env bash
# Diagnose SBOM: run mesonsbom and list all library components.
# Usage: ./scripts/diagnose_sbom.sh <build-dir>
set -eu

BUILD_DIR="${1:-.}"
EXE="${MESONSBOM_EXE:-./build/mesonsbom}"

[ -x "$EXE" ] || { echo "ERROR: mesonsbom not at $EXE"; exit 1; }

OUTPUT=$(mktemp /tmp/sbom_diag_XXXXXX.json)
trap 'rm -f "$OUTPUT"' EXIT

echo "=== Generating SBOM from: $BUILD_DIR ==="
"$EXE" --build-dir "$BUILD_DIR" --output "$OUTPUT"
echo ""

# Write the python script to a temp file
PYSCRIPT=$(mktemp /tmp/sbom_diag_XXXXXX.py)
trap 'rm -f "$OUTPUT" "$PYSCRIPT"' EXIT

cat > "$PYSCRIPT" << 'PYEOF'
import json, sys

try:
    with open(sys.argv[1]) as f:
        sbom = json.load(f)
except Exception as e:
    print(f"ERROR: {e}")
    sys.exit(1)

print(f"BomFormat: {sbom.get('bomFormat')}  SpecVersion: {sbom.get('specVersion')}\n")

libs = [c for c in sbom.get('components', []) if c.get('type') == 'library']
print(f"Library components ({len(libs)}):")
print(f"{'Name':30s} {'Version':25s} {'OS':12s} {'Supplier':20s} Evidence")
print("-" * 100)
for c in libs:
    n = c.get('name','')
    v = c.get('version','')
    has_os = 'YES' if 'evidence' in c else 'no'
    sup = c.get('supplier',{}).get('name','')
    ev_info = ''
    if 'evidence' in c:
        pkgcfg_names = []
        for ident in c.get('evidence',{}).get('identity',[]):
            if ident.get('field') == 'name':
                pkgcfg_names.append(ident.get('concludedValue','?'))
        ev_info = f" [{', '.join(pkgcfg_names)}]" if pkgcfg_names else ''
    print(f"{n:30s} {v:25s} {has_os:12s} {sup:20s}{ev_info}")

unknowns = [c for c in libs if 'unknown' in c.get('version','')]
if unknowns:
    print(f"\nWARNING: {len(unknowns)} components have 'unknown' version:")
    for c in unknowns:
        print(f"   {c['name']}")
else:
    print("\nOK: No 'unknown' versions")

refs = {}
for c in sbom.get('components',[]):
    ref = c.get('bom-ref','')
    refs[ref] = refs.get(ref,0) + 1
dups = {r:n for r,n in refs.items() if n > 1}
if dups:
    print(f"ERROR: Duplicate bom-refs: {dups}")
else:
    print("OK: No duplicate bom-refs")

# Show any dependency edges with duplicate targets
if sbom.get('dependencies'):
    for dep in sbom['dependencies']:
        ref = dep.get('ref','')
        targets = dep.get('dependsOn',[])
        seen = set()
        dcount = 0
        for t in targets:
            if t in seen:
                dcount += 1
            seen.add(t)
        if dcount > 0:
            print(f"WARNING: {ref} has {dcount} duplicate dependsOn entries")
PYEOF

python3 "$PYSCRIPT" "$OUTPUT"