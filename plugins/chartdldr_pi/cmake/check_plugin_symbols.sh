#!/bin/sh
# Fail the build if the plugin dylib references unresolved ChartDldr symbols.
#
# Usage:
#   check_plugin_symbols.sh <dylib>   Scan a Mach-O dylib's undefined symbols
#                                      and exit non-zero if any belong to the
#                                      plugin (indicating a link boundary bug).
#   check_plugin_symbols.sh --extract  Read `nm -u`-style lines from stdin and
#                                      print the offending plugin symbols. Used
#                                      by the fixture test so the parsing can be
#                                      exercised without a real dylib.
set -e

# Normalize `nm -u` output to demangled names and print those owned by the
# plugin (ChartDldr/chartdldr). Mach-O prefixes every symbol with a leading
# underscore, so strip exactly one before demangling: `__ZN9ChartDldr..` becomes
# `_ZN9ChartDldr..` (which c++filt demangles to `ChartDldr::..`) and an
# extern "C" `_ChartDldrFoo` becomes `ChartDldrFoo`. Without stripping that
# underscore first, the anchored grep silently misses every symbol.
extract_plugin_symbols() {
  awk '{ print $NF }' \
    | sed -E 's/^_//' \
    | c++filt \
    | grep -E '^(ChartDldr|chartdldr)' \
    || true
}

if [ "$1" = "--extract" ]; then
  extract_plugin_symbols
  exit 0
fi

dylib="$1"
undefined=$(nm -u "$dylib" 2>/dev/null | extract_plugin_symbols)
if [ -n "$undefined" ]; then
  echo "$dylib has unresolved plugin symbols:"
  echo "$undefined"
  exit 1
fi
