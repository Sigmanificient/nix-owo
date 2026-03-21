#!/bin/sh
set -euo pipefail

nix_hash_path="$1"

run_test () {
  path="$1"
  expected=$(nix-hash --sri "$1" --type sha256)

  echo -n "Testing $1 - "

  result="$($nix_hash_path "$path" 2>/dev/null)"
  if [ "$result" != "$expected" ]; then
    echo "KO"
    echo "expected: $expected"
    echo "got:      $result"
    exit 1
  fi

  echo "OK"
}

run_test "."
run_test "src"
