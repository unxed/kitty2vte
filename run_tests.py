#!/usr/bin/env python3
import subprocess
import itertools
import os
import json
import sys
import argparse
from collections import defaultdict

# --- Configuration ---
KITTY_TESTER = "./build/bin/kitty_tester"
VTE_TESTER = "./build/bin/vte_tester"
RESULTS_FILE = "test_results.json"
MISMATCH_LOG_FILE = "mismatches.log"
SAVE_INTERVAL = 100
COMMAND_TIMEOUT = 2

# Standard X11/evdev keycodes for US QWERTY layout
key_map = {
    'a': {'name': 'a', 'keycode': 38}, 'b': {'name': 'b', 'keycode': 56}, 'c': {'name': 'c', 'keycode': 54},
    'd': {'name': 'd', 'keycode': 40}, 'e': {'name': 'e', 'keycode': 26}, 'f': {'name': 'f', 'keycode': 41},
    'g': {'name': 'g', 'keycode': 42}, 'h': {'name': 'h', 'keycode': 43}, 'i': {'name': 'i', 'keycode': 31},
    'j': {'name': 'j', 'keycode': 44}, 'k': {'name': 'k', 'keycode': 45}, 'l': {'name': 'l', 'keycode': 46},
    'm': {'name': 'm', 'keycode': 58}, 'n': {'name': 'n', 'keycode': 57}, 'o': {'name': 'o', 'keycode': 32},
    'p': {'name': 'p', 'keycode': 33}, 'q': {'name': 'q', 'keycode': 24}, 'r': {'name': 'r', 'keycode': 27},
    's': {'name': 's', 'keycode': 39}, 't': {'name': 't', 'keycode': 28}, 'u': {'name': 'u', 'keycode': 30},
    'v': {'name': 'v', 'keycode': 55}, 'w': {'name': 'w', 'keycode': 25}, 'x': {'name': 'x', 'keycode': 53},
    'y': {'name': 'y', 'keycode': 29}, 'z': {'name': 'z', 'keycode': 52},

    # Numbers row
    '1': {'name': '1', 'keycode': 10}, '2': {'name': '2', 'keycode': 11}, '3': {'name': '3', 'keycode': 12},
    '4': {'name': '4', 'keycode': 13}, '5': {'name': '5', 'keycode': 14}, '6': {'name': '6', 'keycode': 15},
    '7': {'name': '7', 'keycode': 16}, '8': {'name': '8', 'keycode': 17}, '9': {'name': '9', 'keycode': 18},
    '0': {'name': '0', 'keycode': 19},

    # Top row symbols
    '`': {'name': '`', 'keycode': 49}, '~': {'name': '~', 'keycode': 49},
    '-': {'name': 'minus', 'keycode': 20}, '_': {'name': '_', 'keycode': 20},
    '=': {'name': 'equal', 'keycode': 21}, '+': {'name': '+', 'keycode': 21},
    # Brackets & Slashes
    '[': {'name': 'bracketleft', 'keycode': 34}, '{': {'name': '{', 'keycode': 34},
    ']': {'name': 'bracketright', 'keycode': 35}, '}': {'name': '}', 'keycode': 35},
    '\\': {'name': 'backslash', 'keycode': 51}, '|': {'name': '|', 'keycode': 51},
    ';': {'name': 'semicolon', 'keycode': 47}, ':': {'name': ':', 'keycode': 47},
    "'": {'name': 'apostrophe', 'keycode': 48}, '"': {'name': '"', 'keycode': 48},
    ',': {'name': 'comma', 'keycode': 59}, '<': {'name': '<', 'keycode': 59},
    '.': {'name': 'period', 'keycode': 60}, '>': {'name': '>', 'keycode': 60},
    '/': {'name': 'slash', 'keycode': 61}, '?': {'name': '?', 'keycode': 61},

    # Function keys (F1=67 in evdev)
    **{f'F{i}': {'name': f'F{i}', 'keycode': 66 + i} for i in range(1, 13)},

    # Control keys
    'Escape': {'name': 'Escape', 'keycode': 9},
    'Tab': {'name': 'Tab', 'keycode': 23},
    'Return': {'name': 'Return', 'keycode': 36},
    'BackSpace': {'name': 'BackSpace', 'keycode': 22},
    'space': {'name': 'space', 'keycode': 65},

    # Navigation
    'Insert': {'name': 'Insert', 'keycode': 118},
    'Delete': {'name': 'Delete', 'keycode': 119},
    'Home': {'name': 'Home', 'keycode': 110},
    'End': {'name': 'End', 'keycode': 115},
    'Page_Up': {'name': 'Page_Up', 'keycode': 112},
    'Page_Down': {'name': 'Page_Down', 'keycode': 117},

    # Arrows
    'Up': {'name': 'Up', 'keycode': 111},
    'Down': {'name': 'Down', 'keycode': 116},
    'Left': {'name': 'Left', 'keycode': 113},
    'Right': {'name': 'Right', 'keycode': 114},

    # Keypad (NumLock ON usually implies these codes)
    **{f'KP_{i}': {'name': f'KP_{i}', 'keycode': 86 - (9-i) if i!=0 else 90} for i in range(10)},
    'KP_Home': {'name': 'KP_Home', 'keycode': 79},
    'KP_End': {'name': 'KP_End', 'keycode': 87},
}

def format_raw_output(raw_bytes):
    if not raw_bytes:
        return "[EMPTY]"
    try:
        text = raw_bytes.decode('utf-8', errors='replace')
        return text.replace('\x1b', 'ESC').replace('\x00', '\\0').replace('\r', '\\r').replace('\t', '\\t').replace('\b', '\\b')
    except Exception:
        return f"[DECODE_ERROR: {raw_bytes.hex()}]"

def run_command(cmd_args, debug=False):
    if debug:
        print(f"\n[DEBUG] Running: {' '.join(cmd_args)}", file=sys.stderr)
    try:
        result = subprocess.run(cmd_args, capture_output=True, timeout=COMMAND_TIMEOUT, check=True)
        if debug and result.stderr:
            print(f"[DEBUG] Stderr: {result.stderr.strip().decode('utf-8', 'replace')}", file=sys.stderr)
        return result.stdout.strip()
    except subprocess.TimeoutExpired:
        return f"[ERROR: Command timed out after {COMMAND_TIMEOUT}s]".encode()
    except subprocess.CalledProcessError as e:
        stderr_msg = e.stderr.strip().decode('utf-8', 'replace') if e.stderr else 'N/A'
        return f"[ERROR: Exit code {e.returncode}, Stderr: {stderr_msg}]".encode()
    except Exception as e:
        return f"[ERROR: {str(e)}]".encode()

def format_key_combo(key_info, mods, locks, flags):
    combo_parts = [m.replace('--', '') for m in mods + locks]
    combo_parts.append(key_info['name'])
    combo_str = "+".join(combo_parts)
    return f"Key: {combo_str}, Flags: {flags}"

def save_results(results):
    mismatches = [r for r in results if r['status'] == 'mismatch']

    with open(RESULTS_FILE, 'w') as f:
        json_results = []
        for r in results:
            json_r = r.copy()
            json_r['kitty_out_fmt'] = format_raw_output(json_r.pop('kitty_out'))
            json_r['vte_out_fmt'] = format_raw_output(json_r.pop('vte_out'))
            json_results.append(json_r)
        json.dump(json_results, f, indent=2)

    with open(MISMATCH_LOG_FILE, 'w') as f:
        f.write(f"Found {len(mismatches)} mismatches.\n\n")

        max_combo_len = 0
        max_kitty_len = 0
        if mismatches:
            max_combo_len = max(len(r['combo']) for r in mismatches)
            max_kitty_len = max(len(format_raw_output(r['kitty_out'])) for r in mismatches)

        for item in mismatches:
            combo_str = item['combo'].ljust(max_combo_len)
            kitty_str = format_raw_output(item['kitty_out']).ljust(max_kitty_len)
            vte_str = format_raw_output(item['vte_out'])
            f.write(f"{combo_str} -> kitty: {kitty_str} | vte: {vte_str}\n")

def main():
    parser = argparse.ArgumentParser(description="Test and compare kitty and VTE key encoders.")
    parser.add_argument("--debug", action="store_true", help="Enable debug output for commands.")
    parser.add_argument("--limit", type=int, default=0, help="Limit the number of test combinations to run.")
    args = parser.parse_args()

    if not all(os.path.exists(p) for p in [KITTY_TESTER, VTE_TESTER]):
        print("Error: Testers not found. Please run 'make' first.", file=sys.stderr)
        sys.exit(1)

    mods_to_test = [[]] + [list(c) for i in range(1, 4) for c in itertools.combinations(['--shift', '--ctrl', '--alt'], i)]
    locks_to_test = [ [], ['--caps'], ['--num'], ['--caps', '--num'] ]
    kitty_flags_to_test = range(32)

    keys_to_test = list(key_map.values())

    all_combinations = list(itertools.product(keys_to_test, mods_to_test, locks_to_test, kitty_flags_to_test))
    total_tests = len(all_combinations)
    if args.limit > 0:
        all_combinations = all_combinations[:args.limit]
        total_tests = len(all_combinations)

    print(f"Starting tests. Combinations to check: {total_tests}")

    results = []
    mismatch_count = 0
    save_counter = 0

    key_status = defaultdict(lambda: True)

    try:
        for i, (key_info, mods, locks, flags) in enumerate(all_combinations):

            key_name = key_info['name']

            if i > 0 and i % 500 == 0:
                percent = (i * 100) // total_tests
                print(f"Progress: {percent}% ({i}/{total_tests}) | Found {mismatch_count} mismatches", flush=True)

            base_cmd = ['--key', key_info['name']] + mods + locks
            kitty_cmd = [KITTY_TESTER] + base_cmd + ['--kitty-flags', str(flags)]
            vte_cmd = [VTE_TESTER] + base_cmd + ['--keycode', str(key_info['keycode']), '--kitty-flags', str(flags)]

            kitty_out_raw = run_command(kitty_cmd, args.debug)
            vte_out_raw = run_command(vte_cmd, args.debug)

            kitty_out_str = format_raw_output(kitty_out_raw)
            vte_out_str = format_raw_output(vte_out_raw)

            status = 'error' # Default to error

            if "[ERROR:" in kitty_out_str or "[ERROR:" in vte_out_str:
                status = 'error'
            elif kitty_out_str == "[EMPTY]":
                # Likely kitty shortcut or consumed event, skip comparison
                status = 'skipped_kitty_empty'
            elif vte_out_str == "[LEGACY_FALLBACK]" or vte_out_str == "[EMPTY]":
                status = 'skipped_vte'
            elif kitty_out_str == vte_out_str:
                status = 'match'
            else:
                status = 'mismatch'
                mismatch_count += 1

            if status != 'match':
                key_status[key_name] = False

            test_case = {
                'combo': format_key_combo(key_info, mods, locks, flags),
                'status': status,
                'kitty_out': kitty_out_raw,
                'vte_out': vte_out_raw,
            }
            results.append(test_case)

            if i > 0 and i % (SAVE_INTERVAL * 20) == 0:
                save_counter += 1
                print(f" [Save #{save_counter}] Saving intermediate results...")
                save_results(results)

    except KeyboardInterrupt:
        print("\nTest interrupted by user. Saving current results.")
    finally:
        sys.stdout.write("\n")
        print("Saving final results...")
        save_results(results)

        matches = len([r for r in results if r['status'] == 'match'])
        mismatches = len([r for r in results if r['status'] == 'mismatch'])
        skipped = len([r for r in results if r['status'].startswith('skipped')])
        errors = len([r for r in results if r['status'] == 'error'])

        total_keys_tested = len(key_status)
        successful_keys_count = sum(1 for passed in key_status.values() if passed)

        print("\n--- Test Summary ---")
        print(f"Total combinations run: {len(results)} / {total_tests}")
        print(f"  Matching combinations: {matches}")
        print(f"  Mismatched combinations: {mismatches}")
        print(f"  Skipped (VTE legacy/Kitty empty): {skipped}")
        print(f"  Errors: {errors}")
        print("\n--- Key-based Summary ---")
        print(f"Total unique keys tested: {total_keys_tested}")
        print(f"Keys with ALL combinations matching: {successful_keys_count} / {total_keys_tested}")

        print("\n--- Output Files ---")
        if mismatches or errors:
            print(f"Mismatch/Error details have been logged to '{MISMATCH_LOG_FILE}'")
        print(f"Full raw results are in '{RESULTS_FILE}'")

if __name__ == "__main__":
    main()