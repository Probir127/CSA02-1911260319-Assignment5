#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build/data
CC=${CC:-gcc}
CFLAGS=(-std=c11 -Wall -Wextra -Wpedantic -Werror -O0 -g)
CORE=(common/bank.c common/storage.c common/date.c)
"$CC" --version > compiler.txt
for pair in 'TaskA1/demo:a1' 'TaskA2/design_check:a2' 'TaskA5/storage_check:a5' 'TaskA3/pointers:a3' 'TaskA4/growth:a4' 'TaskB1/recursion:recursion' 'TaskB2/main:bloodbank' 'TaskB2/safe_name:safe_name' 'TaskB2/validation:validation' 'TaskB3/benchmark:benchmark' 'TaskB4/report:report'; do
    source=${pair%:*}; target=${pair#*:}
    "$CC" "${CFLAGS[@]}" -Icommon "$source.c" "${CORE[@]}" -o "build/$target"
done
for task in B1 B3 B4; do
    "$CC" "${CFLAGS[@]}" "Task$task/complete_code.c" -o "build/complete_$task"
    "build/complete_$task" > "Task$task/complete_output.txt"
done
build/a1 > TaskA1/output.txt
build/a2 > TaskA2/output.txt
build/a5 > TaskA5/output.txt
build/a3 > TaskA3/output.txt
build/a4 > TaskA4/output.txt
build/recursion > TaskB1/output.txt
build/validation > TaskB2/validation_output.txt 2>&1
build/benchmark > TaskB3/output.txt
build/report build/data/donors_100000.tsv 2026-11-11 build/data/next_week.tsv > TaskB4/output.txt
cp build/data/donors_10.tsv data/donors_10.tsv
cp build/data/donors_10.bin data/donors_10.bin
cp build/data/donors_1000.tsv data/donors_1000.tsv
cp build/demo_stock.bin data/stock.bin
head -n 21 build/data/next_week.tsv > TaskB4/report_excerpt.tsv
"$CC" "${CFLAGS[@]}" -fno-optimize-sibling-calls -fno-inline TaskB1/stack_probe.c -o build/stack_probe
(
    ulimit -c 0
    ulimit -s 1024
    echo "Child stack limit: $(ulimit -s) KiB; core dumps disabled; -O0 -fno-optimize-sibling-calls -fno-inline"
    last=0; failed=0
    for n in 1000 2000 4000 8000 16000 32000 64000 128000 256000 512000 1000000; do
        set +e
        build/stack_probe recursive "$n"
        status=$?
        set -e
        echo "exit_status=$status"
        if [ "$status" -ne 0 ]; then failed=$n; break; fi
        last=$n
    done
    echo "highest_tested_pass=$last first_tested_failure=$failed"
    if [ "$failed" -gt 0 ]; then build/stack_probe iterative "$failed"; fi
    build/stack_probe iterative 1000000
) > TaskB1/stack_limit.txt 2>&1
ASAN=(-fsanitize=address,undefined -fno-omit-frame-pointer)
"$CC" "${CFLAGS[@]}" -fsanitize=address -fno-omit-frame-pointer TaskB2/unsafe_name.c -o build/unsafe_name
"$CC" "${CFLAGS[@]}" "${ASAN[@]}" -Icommon TaskB2/safe_name.c "${CORE[@]}" -o build/safe_name_asan
"$CC" "${CFLAGS[@]}" "${ASAN[@]}" -Icommon TaskB2/validation.c "${CORE[@]}" -o build/validation_asan
"$CC" "${CFLAGS[@]}" "${ASAN[@]}" -Icommon TaskA1/demo.c "${CORE[@]}" -o build/demo_asan
"$CC" "${CFLAGS[@]}" "${ASAN[@]}" -Icommon TaskB4/report.c "${CORE[@]}" -o build/report_asan
(
    ulimit -c 0
    set +e
    ASAN_OPTIONS=detect_leaks=0 build/unsafe_name < TaskB2/long_name.txt
    status=$?
    echo "Expected unsafe child exit_status=$status"
    test "$status" -ne 0
) > TaskB2/unsafe_asan.txt 2>&1
expect_status() {
    expected=$1; shift
    printf '\nCOMMAND:'; printf ' %q' "$@"; printf '\n'
    set +e
    "$@"
    actual=$?
    set -e
    echo "exit_status=$actual expected=$expected"
    test "$actual" -eq "$expected"
}
(
    expect_status 2 build/bloodbank
    expect_status 2 build/bloodbank find data/donors_10.tsv O+ 2026-11-11 extra
    expect_status 2 build/bloodbank find data/donors_10.tsv X+ 2026-11-11
    expect_status 2 build/bloodbank issue data/stock.bin A- -2
    expect_status 1 build/bloodbank find build/missing.tsv O+ 2026-11-11
    cp data/stock.bin build/readonly.bin
    chmod 444 build/readonly.bin
    expect_status 1 build/bloodbank issue build/readonly.bin A- 2
    chmod 644 build/readonly.bin
    expect_status 0 build/bloodbank find data/donors_1000.tsv O+ 2026-11-18
    expect_status 0 build/bloodbank issue build/readonly.bin A- 2
    expect_status 0 build/bloodbank stock build/readonly.bin
    expect_status 2 build/safe_name < TaskB2/long_name.txt
) > TaskB2/cli_output.txt 2>&1
(
    export ASAN_OPTIONS=detect_leaks=0
    export UBSAN_OPTIONS=halt_on_error=1
    expect_status 2 build/safe_name_asan < TaskB2/long_name.txt
    expect_status 0 build/validation_asan
    expect_status 0 build/demo_asan
    expect_status 0 build/report_asan build/data/donors_100000.tsv 2026-11-11 build/data/next_week_asan.tsv
    echo 'PASS: corrected paths completed without ASan/UBSan diagnostics. LeakSanitizer disabled on this macOS run; tracked owned allocations returned to zero.'
) > TaskB2/sanitizer_output.txt 2>&1
cmp build/data/next_week.tsv build/data/next_week_asan.tsv
printf 'PASS: all required programs and expected-failure experiments completed.\n'
