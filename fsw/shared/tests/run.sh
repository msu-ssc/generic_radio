#!/usr/bin/env bash
set -euo pipefail
component_root=$(cd "$(dirname "$0")/../../.." && pwd)
test_dir=$(mktemp -d /tmp/iris-driver-tests.XXXXXX)
trap 'rm -rf "$test_dir"' EXIT
includes=(-I"$component_root/fsw/shared/tests/stubs"
          -I"$component_root/fsw/cfs/platform_inc"
          -I"$component_root/fsw/shared")
gcc -std=c99 -Wall -Wextra -Werror -fsanitize=undefined "${includes[@]}" \
    -c "$component_root/fsw/shared/generic_radio_device.c" -o "$test_dir/device.o"
g++ -std=c++11 -Wall -Wextra -Werror -fsanitize=undefined "${includes[@]}" \
    -I"$component_root/sim/inc" "$component_root/fsw/shared/tests/power_driver_test.cpp" \
    "$test_dir/device.o" -o "$test_dir/power_driver_test"
"$test_dir/power_driver_test"
