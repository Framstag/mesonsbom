#! /usr/bin/env bash
# Integration test: verify OS package resolution with dedup and mixed scenarios.
set -eu

EXE="${MESONSBOM_EXE:-./build/mesonsbom}"
CLEANUP="${TEST_CLEANUP:-true}"

[ -x "$EXE" ] || { echo "FAIL: mesonsbom not at $EXE"; exit 1; }

PASS=0; FAIL=0; TOTAL=0
check() {
    TOTAL=$((TOTAL+1))
    if [ "$2" = "true" ] || [ "$2" = "0" ]; then
        echo "  OK $1"; PASS=$((PASS+1))
    else
        echo "  FAIL $1"; FAIL=$((FAIL+1))
    fi
}

setup() {
    local d="$1"; rm -rf "$d"
    mkdir -p "$d/meson-info"
    cat > "$d/meson-info/intro-projectinfo.json" << 'EOF'
{"name": "test-project", "version": "1.0.0", "license": [],
 "descriptive_name": "Test Project"}
EOF
}

has_comp() {
    python3 -c "
import json, sys
d = json.load(open(sys.argv[1]))
sys.exit(0 if any(c.get('name') == sys.argv[2] for c in d.get('components',[])) else 1)
" "$1" "$2" 2>/dev/null
}

has_edge() {
    python3 -c "
import json, sys
d = json.load(open(sys.argv[1]))
for dep in d.get('dependencies',[]):
    if dep.get('ref') == sys.argv[2] and sys.argv[3] in dep.get('dependsOn',[]): sys.exit(0)
sys.exit(1)
" "$1" "$2" "$3" 2>/dev/null
}

# ==========================
# Test 1: nested chain
# ==========================
echo; echo "=== Test 1: Nested chain ==="
TD=$(mktemp -d /tmp/sbt1_XXXXXX)
setup "$TD"
echo '[{"name":"A","version":"1.0.0","type":"library"}]' > "$TD/meson-info/intro-dependencies.json"
mkdir -p "$TD/meson-info/subprojects/A"
echo '[{"name":"B","version":"2.0.0","type":"library"}]' > "$TD/meson-info/subprojects/A/intro-dependencies.json"
mkdir -p "$TD/meson-info/subprojects/B"
echo '[{"name":"C","version":"3.0.0","type":"library"}]' > "$TD/meson-info/subprojects/B/intro-dependencies.json"
mkdir -p "$TD/meson-info/subprojects/C"
echo '[]' > "$TD/meson-info/subprojects/C/intro-dependencies.json"
"$EXE" --build-dir "$TD" --output "$TD/sbom.json" 2>/dev/null

S="$TD/sbom.json"
check "main present"  "$(has_comp "$S" test-project && echo true || echo false)"
check "A present"     "$(has_comp "$S" A && echo true || echo false)"
check "B present"     "$(has_comp "$S" B && echo true || echo false)"
check "C present"     "$(has_comp "$S" C && echo true || echo false)"
check "edge P->A"     "$(has_edge "$S" test-project A && echo true || echo false)"
check "edge A->B"     "$(has_edge "$S" A B && echo true || echo false)"
check "edge B->C"     "$(has_edge "$S" B C && echo true || echo false)"
$CLEANUP && rm -rf "$TD"

# ==========================
# Test 2: Qt-style dedup
# ==========================
echo; echo "=== Test 2: Qt-style dedup ==="
TD=$(mktemp -d /tmp/sbt2_XXXXXX)
setup "$TD"
cat > "$TD/meson-info/intro-dependencies.json" << 'EOF'
[
  {"name":"Qt5Core","version":"5.15.0","type":"pkgconfig"},
  {"name":"Qt5Gui","version":"5.15.0","type":"pkgconfig"},
  {"name":"Qt5Widgets","version":"5.15.0","type":"pkgconfig"}
]
EOF
"$EXE" --build-dir "$TD" --output "$TD/sbom.json" 2>/dev/null

S="$TD/sbom.json"
echo -n "  library count: "
python3 -c "import json,sys; d=json.load(open(sys.argv[1])); print(len([c for c in d.get('components',[]) if c.get('type')=='library']))" "$S" 2>/dev/null
echo -n "  OS-resolved: "
python3 -c "
import json,sys; d=json.load(open(sys.argv[1]))
for c in d.get('components',[]):
    if c.get('type')=='library' and 'evidence' in c:
        print(c['name']); sys.exit(0)
print('none')
" "$S" 2>/dev/null
$CLEANUP && rm -rf "$TD"

# ==========================
# Test 3: single dep
# ==========================
echo; echo "=== Test 3: Single dep ==="
TD=$(mktemp -d /tmp/sbt3_XXXXXX)
setup "$TD"
echo '[{"name":"zlib","version":"1.0.0","type":"pkgconfig"}]' > "$TD/meson-info/intro-dependencies.json"
"$EXE" --build-dir "$TD" --output "$TD/sbom.json" 2>/dev/null

S="$TD/sbom.json"
echo -n "  zlib version: "
python3 -c "
import json,sys; d=json.load(open(sys.argv[1]))
for c in d.get('components',[]):
    if 'zlib' in c.get('name','').lower():
        print(c.get('version','')); sys.exit(0)
print('NOT FOUND')
" "$S" 2>/dev/null
$CLEANUP && rm -rf "$TD"

# ==========================
# Test 4: Meson bridge deps (Qt5Core.dependencies -> Qt5Svg)
# ==========================
echo; echo "=== Test 4: Meson bridge deps ==="
TD=$(mktemp -d /tmp/sbt4_XXXXXX)
mkdir -p "$TD/meson-info"
cat > "$TD/meson-info/intro-projectinfo.json" << 'EOF'
{"name":"myapp","version":"1.0.0","license":[]}
EOF
# Qt5Core has a dependencies field with Qt5Svg (common Meson pattern).
# Qt5Core.pc has no Requires, so pkg-config fallback would return nothing.
# The bridge fallback should discover Qt5Svg from intro-dependencies.json.
cat > "$TD/meson-info/intro-dependencies.json" << 'EOF'
[
  {"name":"Qt5Core","version":"5.15.0","type":"pkgconfig",
   "dependencies":["Qt5Core","Qt5Svg"]}
]
EOF
"$EXE" --build-dir "$TD" --output "$TD/sbom.json" 2>/dev/null
S="$TD/sbom.json"
echo -n "  components: "
python3 -c "
import json,sys; d=json.load(open(sys.argv[1]))
for c in d.get('components',[]):
    if c.get('type')=='library':
        print(c.get('name',''), end=' ')
print()
" "$S" 2>/dev/null
$CLEANUP && rm -rf "$TD"

echo; echo "=== $PASS/$TOTAL passed ==="
[ "$FAIL" -gt 0 ] && exit 1 || exit 0