# CSA02 - C Programming | Assignment 5 (CO5)

**Student:** PROBIR SAHA SHOHOM  
**Register number:** 1911260319  
**Course:** CSA02 - C Programming

[Solution PDF](CSA02_Assignment5_Solution.pdf) | [Full written solutions](SOLUTIONS.md) | [Full experiment data](CSA02_Assignment5_Data.zip)

Modular blood-bank case study: functions, recursion, pointers, dynamic memory, command-line validation and persistent files. All donor data are synthetic. The demonstration rule (age 18-65; 90-day interval) is not a clinical policy.

## Build and reproduce

Compiler used: Apple Clang 21.0.0 (clang-2100.3.34.2), arm64-apple-darwin27.0.0, invoked as `gcc`. See compiler.txt.

```bash
bash tools/run_all.sh
```

The runner includes a deliberately unsafe scanf child and a stack-limited recursion child. Their failures are required experiments. It verifies all other expected statuses and runs the corrected modules with ASan/UBSan. Do not use the unsafe example in the application.

Exact common build pattern:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -O0 -g -Icommon TaskB2/main.c common/bank.c common/storage.c common/date.c -o build/bloodbank
build/bloodbank find data/donors_1000.tsv O+ 2026-11-18
build/bloodbank stock data/stock.bin
```

Create build/data before compiling manually. The complete exact compiler commands for all tasks, sanitizer variants and stack flags are in tools/run_all.sh. No Python runtime is needed.

| Task | C driver (shared modules where applicable) | Run after full build | Result |
|---|---|---|---|
| A1 | TaskA1/demo.c | build/a1 | Registration, donation, issue, two-output interfaces and reload pass |
| A2 | TaskA2/design_check.c | build/a2 | Digital-root trace and zero case pass |
| A3 | TaskA3/pointers.c | build/a3 | Pointer offsets, post-increment and returned pointer pass |
| A4 | TaskA4/growth.c | build/a4 | Failed realloc preserves list; growth costs measured |
| A5 | TaskA5/storage_check.c | build/a5 | One stock record updated; seven others unchanged |
| B1 | TaskB1/recursion.c, complete_code.c, stack_probe.c | build/recursion; build/complete_B1 | Call counts match; stack child first failed at 64000 |
| B2 | TaskB2/main.c, safe_name.c, unsafe_name.c, validation.c | use runner for adversarial cases | Expected errors; corrected paths sanitizer-clean |
| B3 | TaskB3/benchmark.c, complete_code.c | build/benchmark; build/complete_B3 | Four sizes round-trip; 50000th update verified |
| B4 | TaskB4/report.c, complete_code.c | build/report build/data/donors_100000.tsv 2026-11-11 build/data/next_week.tsv | 53852 eligible; 13 MiB reserved; buffers freed |

## Files and evidence

- Task folders: answers, C source, captured stdout/stderr, labelled output PNGs.
- common/: validated donor/stock functions, date functions and persistence.
- data/: 10-record text/binary fixtures, 1000-record text fixture and stock input.
- TaskB3/*.csv: three repeated size/timing observations and one update comparison.
- TaskB2/unsafe_asan.txt: expected stack-buffer-overflow diagnostic; sanitizer_output.txt: corrected executions.
- build/data/: regenerated 10/1000/10000/100000 datasets and full nightly report (ignored by git).
- Data archive: full generated benchmark files and report are included in CSA02_Assignment5_Data.zip supplied with the solution; they can also be reproduced by the C benchmark/runner.

Output PNGs are renderings of actual captured logs, not personal terminal screenshots. The assignment requires individual execution and faculty sharing: run the commands yourself, add your own terminal screenshots, and share this repository with your faculty. All commits use actual dates; the current history does not represent a week of work.

The prototype demonstrates normal file persistence, not crash-atomic or concurrent transactions. Native binary structs require the same ABI. For larger nightly reports, batching avoids retaining the full dataset; an ID index helps random lookups but does not eliminate an eligibility scan. Detailed assumptions, measurements and alternatives appear in SOLUTIONS.md.
