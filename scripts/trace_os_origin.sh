#! /usr/bin/env bash
# Trace which original Meson dep resolved to a given OS package name.
# Helps debug unexpected OS-resolved components (e.g., qt6-base appearing
# when no Qt6 dependency exists in the project).
#
# Usage: ./scripts/trace_os_origin.sh <sbom.json> [<os-package-name>]
#   Default filter is "qt6"
set -eu

SBOM="${1:?Usage: $0 <sbom.json> [os-package-name]}"
FILTER="${2:-qt6}"

[ -f "$SBOM" ] || { echo "ERROR: $SBOM not found"; exit 1; }

PYSCRIPT=$(mktemp /tmp/trace_XXXXXX.py)
trap 'rm -f "$PYSCRIPT"' EXIT

cat > "$PYSCRIPT" << 'PYEOF'
import json, sys

with open(sys.argv[1]) as f:
    sbom = json.load(f)

filter_name = sys.argv[2].lower()
found = False

for c in sbom.get('components', []):
    name = c.get('name', '')
    if filter_name not in name.lower():
        continue
    found = True
    ev = c.get('evidence', {})
    identities = ev.get('identity', [])

    print(f'Component: {c.get("name","?")}  version: {c.get("version","?")}')
    print(f'  purl: {c.get("purl","?")}')
    print(f'  type: {c.get("type","?")}')
    print(f'  supplier: {c.get("supplier",{}).get("name","none")}')

    pkgcfg_sources = []
    for ident in identities:
        if ident.get('field') == 'name':
            val = ident.get('concludedValue', '')
            method_val = ''
            for m in ident.get('methods', []):
                method_val = m.get('value', '')
            pkgcfg_sources.append((val, method_val))

    if pkgcfg_sources:
        print(f'  Originated from pkg-config dep(s):')
        for src_name, method_val in pkgcfg_sources:
            print(f'    - {src_name}')
            print(f'      method: {method_val}')
    else:
        print(f'  NOT OS-resolved (pkg-config fallback)')
    print()

if not found:
    print(f'No component matching "{filter_name}" found in {sys.argv[1]}')
    sys.exit(1)
PYEOF

python3 "$PYSCRIPT" "$SBOM" "$FILTER"