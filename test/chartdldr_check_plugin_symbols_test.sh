#!/bin/sh
# Fixture test for plugins/chartdldr_pi/cmake/check_plugin_symbols.sh.
#
# Feeds representative `nm -u` Mach-O undefined-symbol lines through the script's
# --extract mode and asserts that plugin symbols (including leading-underscore
# mangling and extern "C" names) are reported while unrelated system/C++ symbols
# are not. Guards against regressing the underscore-stripping step that lets the
# anchored grep actually match Mach-O names.
set -eu

script="$1"

fixture=$(cat <<'EOF'
                 U __Z12ChartDldrFoov
                 U __ZN18ChartDldrPanelImpl9BulkStateEv
                 U _ChartDldrLegacyEntryPoint
                 U __ZNSt6vectorIiSaIiEE9push_backEOi
                 U _malloc
                 U _OBJC_CLASS_$_NSObject
EOF
)

result=$(printf '%s\n' "$fixture" | /bin/sh "$script" --extract)

fail=0

expect_present() {
  if ! printf '%s\n' "$result" | grep -q "$1"; then
    echo "MISS: expected plugin symbol not detected: $1"
    fail=1
  fi
}

expect_absent() {
  if printf '%s\n' "$result" | grep -qi "$1"; then
    echo "FALSE POSITIVE: unrelated symbol reported: $1"
    fail=1
  fi
}

# Mangled free function, mangled class method, and an extern "C" plugin symbol
# with only the Mach-O leading underscore.
expect_present '^ChartDldrFoo()$'
expect_present '^ChartDldrPanelImpl::BulkState()$'
expect_present '^ChartDldrLegacyEntryPoint$'

# Standard library, libc, and Objective-C runtime symbols must be ignored.
expect_absent 'std::vector'
expect_absent 'malloc'
expect_absent 'NSObject'

if [ "$fail" -eq 0 ]; then
  echo "OK: plugin symbol extraction correct"
fi
exit "$fail"
