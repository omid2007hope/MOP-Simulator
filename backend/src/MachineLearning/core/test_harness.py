#!/usr/bin/env python3
"""
PROJECT AXIOM - Automated Closed-Loop Test Harness
Fuzzes and stress-tests app.exe across 1,000+ CLI argument combinations,
interactive menu edge cases, boundary parameters, and stdin fuzzing.
Records all results and dumps any crashes/hangs/abnormal exits to crash_report.json.
"""

import os
import sys
import json
import time
import string
import random
import datetime
import subprocess

# Configure UTF-8 encoding
sys.stdout.reconfigure(encoding='utf-8')

EXE_PATH = os.path.abspath("app.exe")
CRASH_REPORT_PATH = os.path.abspath("crash_report.json")

# Windows crash exit codes
CRASH_CODES = {
    0xC0000005: "STATUS_ACCESS_VIOLATION (Segmentation Fault)",
    0xC00000FD: "STATUS_STACK_OVERFLOW",
    0xC0000409: "STATUS_STACK_BUFFER_OVERRUN",
    0xC0000028: "STATUS_BAD_STACK",
    0xC000013A: "STATUS_CONTROL_C_EXIT",
    0x80000003: "STATUS_BREAKPOINT",
}

def is_crash_code(code):
    if code is None:
        return False
    # Check negative representation or unsigned 32-bit representation
    u_code = code & 0xFFFFFFFF if code < 0 else code
    if u_code in CRASH_CODES:
        return True
    if u_code >= 0xC0000000:
        return True
    return False

def get_crash_description(code):
    if code is None:
        return "Unknown"
    u_code = code & 0xFFFFFFFF if code < 0 else code
    return CRASH_CODES.get(u_code, f"Windows Exception 0x{u_code:08X}")

def run_test_case(test_id, category, args, stdin_data, timeout_sec=15, expected_exit_codes=None):
    """
    Executes app.exe with specified arguments and stdin.
    Returns (status, result_dict)
    """
    start_time = time.time()
    p = None
    stdout_data = ""
    stderr_data = ""
    timed_out = False
    return_code = None
    exception_str = None

    try:
        p = subprocess.Popen(
            [EXE_PATH] + args,
            stdin=subprocess.PIPE if stdin_data is not None else subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding='utf-8',
            errors='replace'
        )
        stdout_data, stderr_data = p.communicate(input=stdin_data, timeout=timeout_sec)
        return_code = p.returncode
    except subprocess.TimeoutExpired:
        timed_out = True
        if p:
            p.kill()
            stdout_data, stderr_data = p.communicate()
        exception_str = f"Execution timed out after {timeout_sec}s (Process Hang)"
    except Exception as e:
        if p:
            p.kill()
        exception_str = f"Subprocess exception: {str(e)}"
    
    elapsed = time.time() - start_time

    # Determine pass/fail
    is_crash = False
    failure_reason = None

    if timed_out:
        is_crash = True
        failure_reason = exception_str
    elif return_code is not None and is_crash_code(return_code):
        is_crash = True
        failure_reason = f"Fatal Crash: {get_crash_description(return_code)} (code {return_code})"
    elif expected_exit_codes is not None and return_code not in expected_exit_codes:
        is_crash = True
        failure_reason = f"Unexpected exit code {return_code} (expected {expected_exit_codes})"

    record = {
        "test_id": test_id,
        "category": category,
        "args": args,
        "stdin_snippet": (stdin_data[:100] + "...") if (stdin_data and len(stdin_data) > 100) else stdin_data,
        "exit_code": return_code,
        "elapsed_sec": round(elapsed, 4),
        "is_crash": is_crash,
        "failure_reason": failure_reason,
        "stdout_tail": stdout_data[-300:] if stdout_data else "",
        "stderr_tail": stderr_data[-300:] if stderr_data else ""
    }

    return not is_crash, record

def generate_test_cases():
    """Generates 1,000+ diverse test cases across all categories."""
    cases = []
    tid = 1

    # =========================================================================
    # Category 1: Standard CLI Flags & Execution Modes (20 cases)
    # =========================================================================
    cases.append((tid, "cli_standard", ["--help"], None, 10, [0]))
    tid += 1
    cases.append((tid, "cli_standard", ["-h"], None, 10, [0]))
    tid += 1
    cases.append((tid, "cli_standard", ["--test"], None, 30, [0]))
    tid += 1
    cases.append((tid, "cli_standard", ["--benchmark"], None, 30, [0]))
    tid += 1
    cases.append((tid, "cli_standard", ["--train", "5"], None, 25, [0]))
    tid += 1
    cases.append((tid, "cli_standard", ["--train-bulk", "5"], None, 25, [0]))
    tid += 1
    cases.append((tid, "cli_standard", ["--bulk", "5"], None, 25, [0]))
    tid += 1
    cases.append((tid, "cli_standard", ["--use", "--weights", "weights.axiom"], "exit\n", 15, [0]))
    tid += 1
    cases.append((tid, "cli_standard", ["--use", "--vocab", "vocab.axiom"], "exit\n", 15, [0]))
    tid += 1
    cases.append((tid, "cli_standard", ["--use", "--weights=weights.axiom", "--vocab=vocab.axiom"], "exit\n", 15, [0]))
    tid += 1

    # Create a dedicated sandbox directory for archive testing to protect asset_sample/
    sandbox_dir = os.path.abspath("test_sandbox_assets")
    os.makedirs(sandbox_dir, exist_ok=True)
    with open(os.path.join(sandbox_dir, "test_doc.txt"), "w", encoding="utf-8") as sf:
        sf.write("Sandbox training document for Project Axiom archive validation.\n" * 10)

    # Learning rate flag variations
    cases.append((tid, "cli_standard", ["--train", "2", "--lr", "0.005"], None, 20, [0]))
    tid += 1
    cases.append((tid, "cli_standard", ["--train", "2", "--lr=0.002"], None, 20, [0]))
    tid += 1
    cases.append((tid, "cli_standard", ["--data", sandbox_dir, "--bulk", "2", "--archive"], None, 20, [0]))
    tid += 1

    # =========================================================================
    # Category 2: CLI Flag Edge Cases & Syntax Validation Rejections (80 cases)
    # Invalid flags must exit cleanly with code 1 (graceful error), NEVER crash
    # =========================================================================
    bad_cli_args = [
        ["--unknown"],
        ["-x"],
        ["--lr"],                           # missing value
        ["--lr", "invalid_float"],          # non-numeric
        ["--lr", "-0.01"],                  # negative rate
        ["--lr", "0.0"],                    # zero rate
        ["--lr=-1.0"],                      # negative via =
        ["--lr=abc"],                       # non-numeric via =
        ["--weights"],                      # missing value
        ["--vocab"],                        # missing value
        ["--data"],                         # missing value
        ["--weights="],                     # empty value
        ["--vocab="],                       # empty value
        ["--data="],                        # empty value
        ["--weights", "non_existent_dir/nested/missing.axiom", "--use"],  # fallback to baseline
        ["--vocab", "non_existent_vocab.axiom", "--use"],
        ["--train", "-5"],                  # negative step
        ["--train", "0"],                   # zero step
        ["--bulk", "-1"],
        ["--train", "abc"],                 # invalid step string
        ["--train", "9999999999999999999999999999999999999999999999999999999999999999"], # overflow
        ["--data", "invalid_dir_that_does_not_exist_xyz123", "--bulk", "1"],
        ["--archive", "--purge"],           # multiple flags
        ["--help", "--unknown"],            # help takes precedence
        ["-h", "--weights", "test"],
        ["", "--help"],                     # empty string arg
        ["   "],                            # whitespace arg
        ["--weights", "path with spaces/weights.axiom", "--use"],
        ["--weights", "special_!@#$%^&*()_weights.axiom", "--use"],
        ["--lr", "1e-4", "--train", "1"],
        ["--lr", "1e-10", "--train", "1"],
        ["--lr", "100.0", "--train", "1"],
    ]

    for bargs in bad_cli_args:
        # Expected code is either 0 or 1, depending on whether it's an error or handled fallback
        cases.append((tid, "cli_syntax_edge", bargs, "exit\nq\n\n", 15, [0, 1]))
        tid += 1

    # Fuzz CLI arguments with random strings and symbols (100 cases)
    for _ in range(100):
        rand_flag = "--" + "".join(random.choices(string.ascii_letters + string.digits + "_-", k=random.randint(1, 20)))
        rand_val = "".join(random.choices(string.printable, k=random.randint(0, 30)))
        cases.append((tid, "cli_fuzz", [rand_flag, rand_val], "6\nexit\nq\n", 10, [0, 1]))
        tid += 1

    # =========================================================================
    # Category 3: Interactive Menu Navigation & Scenarios (50 cases)
    # =========================================================================
    # Menu Option 6: Exit
    cases.append((tid, "interactive_menu", [], "6\n", 10, [0]))
    tid += 1

    # Invalid menu choices (must handle gracefully without crashing, code 0)
    for invalid_opt in ["0", "7", "-1", "9999", "-9999", "abc", "exit", "!@#$", "\n", "   \n"]:
        cases.append((tid, "interactive_menu_invalid", [], f"{invalid_opt}\n", 10, [0, 1]))
        tid += 1

    # Inference mode prompts (Option 1)
    inference_inputs = [
        "1\nexit\n",
        "1\nquit\n",
        "1\n:q\n",
        "1\nHello World\nexit\n",
        "1\nProject Axiom\nDeep Learning\nCUTLASS\nexit\n",
        "1\n12345 67890\nexit\n",
        "1\n!@#$%^&*()_+=-`~[]\\{}|;':\",./<>?\nexit\n",
        "1\n   \n\n\n\nexit\n",
        "1\n" + ("A" * 500) + "\nexit\n",
        "1\n" + ("word " * 200) + "\nexit\n",
        "1\nمرحبا بك في اكسيوم\nexit\n",        # Arabic UTF-8
        "1\n你好世界\nexit\n",                   # Chinese UTF-8
        "1\nسلام بر اکسیوم\nexit\n",            # Persian UTF-8
        "1\nПривет мир\nexit\n",               # Russian UTF-8
        "1\n🚀🔥🤖💻✨\nexit\n",                # Emojis
    ]
    for inp in inference_inputs:
        cases.append((tid, "interactive_inference", [], inp, 20, [0]))
        tid += 1

    # Training mode edge cases (Option 2)
    training_inputs = [
        # Option 2 -> Text (1) -> Sentence -> No Media (2) -> q
        ("training_text_no_media", "2\n1\nInteractive text training sample\n2\nq\n"),
        # Option 2 -> Text (1) -> Empty/Whitespace retried then valid -> 2 -> q
        ("training_text_retry", "2\n1\n   \nValid sentence\n2\nq\n"),
        # Option 2 -> Text (1) -> Sentence -> Media (1) -> Image (1) -> Photo.jpeg -> q
        ("training_text_image", "2\n1\nText with photo\n1\n1\nPhoto.jpeg\nq\n"),
        # Option 2 -> Text (1) -> Sentence -> Media (1) -> Video (2) -> Video.mp4 -> q
        ("training_text_video", "2\n1\nText with video\n1\n2\nVideo.mp4\nq\n"),
        # Option 2 -> Text (1) -> Sentence -> Media (1) -> Voice (3) -> Voice.ogg -> q
        ("training_text_voice", "2\n1\nText with audio\n1\n3\nVoice.ogg\nq\n"),
        # Option 2 -> Text (1) -> Sentence -> Media (1) -> Invalid media type (9) -> exit
        ("training_text_invalid_media", "2\n1\nText with invalid media\n1\n9\n"),
        # Option 2 -> Number (2) -> Numbers -> No Media (2) -> q
        ("training_number_no_media", "2\n2\n10.5 20.25 30.75 40.0\n2\nq\n"),
        # Option 2 -> Number (2) -> Invalid non-number retry -> Valid -> 2 -> q
        ("training_number_retry", "2\n2\nabc\n1.0 2.0 3.0\n2\nq\n"),
        # Option 2 -> Image Only (3) -> Photo.jpeg -> q
        ("training_image_only", "2\n3\nPhoto.jpeg\nq\n"),
        # Option 2 -> Video Only (4) -> Video.mp4 -> q
        ("training_video_only", "2\n4\nVideo.mp4\nq\n"),
        # Option 2 -> Voice Only (5) -> Voice.ogg -> q
        ("training_voice_only", "2\n5\nVoice.ogg\nq\n"),
        # Option 2 -> Invalid Data Type (9) -> exit
        ("training_invalid_dtype", "2\n9\n"),
        # Option 2 -> Invalid Media Option (9) -> exit
        ("training_invalid_add_media", "2\n1\nText\n9\n"),
    ]
    for cat, inp in training_inputs:
        cases.append((tid, f"interactive_{cat}", [], inp, 25, [0, 1]))
        tid += 1

    # Bulk Training Menu (Option 5)
    bulk_inputs = [
        # Option 5 -> Training runs -> q (stop) -> Asset Management Keep (1)
        ("bulk_keep", "5\nq\n1\n"),
        # Option 5 -> Training runs -> q (stop) -> Asset Management Purge -> Cancel N (3 -> N)
        ("bulk_purge_cancel", "5\nq\n3\nN\n"),
    ]
    for cat, inp in bulk_inputs:
        cases.append((tid, f"interactive_{cat}", [], inp, 30, [0]))
        tid += 1

    # =========================================================================
    # Category 4: Stdin Boundary & Fuzzing Matrix (750+ cases)
    # Bombard with varied byte sequences, random strings, empty pipes, etc.
    # =========================================================================
    # Empty stdin (immediate EOF)
    cases.append((tid, "stdin_fuzz", [], "", 10, [0, 1]))
    tid += 1

    # Stdin fuzz patterns
    fuzz_patterns = [
        "\x00" * 50,                        # Null bytes
        "\xff" * 50,                        # High bytes
        "A" * 1000,                         # Buffer overflow candidate
        "A" * 5000,
        "A" * 10000,
        "%s%s%s%s%s%s%s%s%s%s",             # Format string candidate
        "../../../../../../../../windows",  # Path traversal
        "\\x00\\x01\\x02\\x03",
        "\r\n\r\n\r\n\r\n\r\n",
        "\t\t\t\t\t\t\t\t",
        "0" * 500,
        "-1" * 500,
        "99999999999999999999999999999999",
        "-99999999999999999999999999999999",
        "NaN NaN Inf -Inf",
        "null undefined None NIL",
        "SELECT * FROM weights WHERE id = 1;", # SQL-like
        "<script>alert(1)</script>",         # HTML/XSS-like
        "{'mode': 'train', 'step': 10}",     # JSON-like
    ]

    for pat in fuzz_patterns:
        cases.append((tid, "stdin_pattern_fuzz", [], pat + "\n", 10, [0, 1]))
        tid += 1

    # Fill up to 1,000 total cases with randomized stdin streams and randomized CLI flags
    while len(cases) < 1000:
        rnd_type = random.randint(1, 4)
        if rnd_type == 1:
            # Random string stdin to menu
            rand_str = "".join(random.choices(string.printable, k=random.randint(1, 150)))
            cases.append((tid, "random_stdin_fuzz", [], rand_str + "\n", 10, [0, 1]))
        elif rnd_type == 2:
            # Random menu navigation sequence
            seq = [str(random.randint(-5, 10)) for _ in range(random.randint(2, 6))]
            timeout = 35 if seq[0] in ["3", "4", "5"] else 12
            cases.append((tid, "random_menu_nav", [], "\n".join(seq) + "\n", timeout, [0, 1]))
        elif rnd_type == 3:
            # Random CLI flag combination with valid exit
            rand_flag = "--" + "".join(random.choices(string.ascii_lowercase, k=random.randint(3, 8)))
            cases.append((tid, "random_cli_flag", [rand_flag], "6\n", 10, [0, 1]))
        else:
            # Unicode and multibyte fuzzing
            unicode_chars = [chr(random.randint(0x0080, 0xFFFF)) for _ in range(random.randint(5, 50))]
            cases.append((tid, "random_unicode_fuzz", [], "".join(unicode_chars) + "\n", 10, [0, 1]))
        tid += 1

    return cases

def ensure_sample_assets():
    """Guarantees asset_sample has its baseline multimodal files."""
    asset_dir = os.path.abspath("asset_sample")
    archive_dir = os.path.join(asset_dir, "archive")
    if os.path.isdir(archive_dir):
        for f in os.listdir(archive_dir):
            src = os.path.join(archive_dir, f)
            dst = os.path.join(asset_dir, f)
            if os.path.isfile(src) and not os.path.exists(dst):
                try:
                    os.rename(src, dst)
                except Exception:
                    pass

def main():
    print("=" * 70)
    print("  PROJECT AXIOM — AUTOMATED CLOSED-LOOP TEST HARNESS")
    print(f"  Target Executable: {EXE_PATH}")
    print(f"  Start Timestamp  : {datetime.datetime.now().isoformat()}")
    print("=" * 70)

    if not os.path.exists(EXE_PATH):
        print(f"[!] Target binary not found: {EXE_PATH}")
        sys.exit(1)

    ensure_sample_assets()

    print("\n[..] Generating 1,000+ randomized and edge-case test combinations...")
    cases = generate_test_cases()
    total_cases = len(cases)
    print(f"[OK] Generated {total_cases} distinct test cases.\n")

    passed_count = 0
    failed_count = 0
    crashes = []
    category_stats = {}

    start_suite_time = time.time()
    
    # Progress interval
    report_interval = 100

    print(f"[..] Bombarding {os.path.basename(EXE_PATH)} across all scenarios...")
    for idx, (tid, category, args, stdin_data, timeout_sec, expected_codes) in enumerate(cases, 1):
        if category not in category_stats:
            category_stats[category] = {"total": 0, "passed": 0, "failed": 0}
        category_stats[category]["total"] += 1

        success, record = run_test_case(
            test_id=tid,
            category=category,
            args=args,
            stdin_data=stdin_data,
            timeout_sec=timeout_sec,
            expected_exit_codes=expected_codes
        )

        if success:
            passed_count += 1
            category_stats[category]["passed"] += 1
        else:
            failed_count += 1
            category_stats[category]["failed"] += 1
            crashes.append(record)
            print(f"\n  [ALERT] Test #{tid} ({category}) FAILED: {record['failure_reason']}")
            if record['args']:
                print(f"          Args : {record['args']}")
            if record['stdin_snippet']:
                print(f"          Stdin: {record['stdin_snippet']}")

        if idx % report_interval == 0 or idx == total_cases:
            pct = (idx / total_cases) * 100.0
            elapsed = time.time() - start_suite_time
            print(f"  [Progress] {idx}/{total_cases} ({pct:5.1f}%) | Passed: {passed_count} | Crashes: {failed_count} | Elapsed: {elapsed:.1f}s", flush=True)

    total_duration = time.time() - start_suite_time

    # Compile crash report
    report_data = {
        "timestamp": datetime.datetime.now().isoformat(),
        "executable": EXE_PATH,
        "total_tests": total_cases,
        "passed": passed_count,
        "failed": failed_count,
        "duration_sec": round(total_duration, 2),
        "category_summary": category_stats,
        "crashes": crashes
    }

    with open(CRASH_REPORT_PATH, "w", encoding="utf-8") as f:
        json.dump(report_data, f, indent=2)

    print("\n" + "=" * 70)
    print("  TEST HARNESS EXECUTION SUMMARY")
    print("=" * 70)
    print(f"  Total Scenarios Executed : {total_cases}")
    print(f"  Passed Without Crash     : {passed_count}")
    print(f"  Crashes / Errors Detected: {failed_count}")
    print(f"  Total Duration           : {total_duration:.2f}s")
    print(f"  Crash Report Written To  : {CRASH_REPORT_PATH}")
    print("----------------------------------------------------------------------")
    print("  Category Breakdown:")
    for cat, stat in category_stats.items():
        print(f"    - {cat:<26}: {stat['passed']}/{stat['total']} passed ({stat['failed']} failed)")
    print("=" * 70)

    if failed_count > 0:
        print(f"\n[!] FAILURE: {failed_count} crashes or unexpected failures logged to crash_report.json.")
        sys.exit(1)
    else:
        print("\n[OK] 100% SUCCESS: All 1,000 scenarios passed cleanly with zero crashes or hangs!")
        sys.exit(0)

if __name__ == "__main__":
    main()
