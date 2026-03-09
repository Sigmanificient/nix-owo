#!/usr/bin/env bash
set -euo pipefail

run_test () {
  name="$1"
  path="$2"
  expected="$3"

  echo "Testing $name"
  start=$(date +%s%N)

  result="$(./nix-hash "$path" 2>/dev/null)"
  if [ "$result" != "$expected" ]; then
    echo "Mismatch on iteration $i"
    echo "expected: $expected"
    echo "got:      $result"
    exit 1
  fi

  end=$(date +%s%N)
  elapsed_ns=$((end - start))
  elapsed_ms=$((elapsed_ns / 1000000))

  echo "OK ($name) - ${elapsed_ms} ms for 100 runs"
  echo
}

run_test "FILTERPATH" "$FILTERPATH_SOURCE_TEST" "sha256-FOewYznmWOWH2TyNySVoa+spvH4QlXnjlko+/zFiNik="
run_test "CRITERION" "$CRITERION_SOURCE_TEST" "sha256-X4m/uCyanS7HLtf6GyK4XuaT5i+HQt1PZC7gd813IVQ="
run_test "QTILE" "$QTILE_SOURCE_TEST" "sha256-PPyI+IGvHBQusVmU3D26VjYjLaa9+94KUqNwbQSzeaI="
