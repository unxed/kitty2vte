#!/usr/bin/env python3
import subprocess
import itertools
import os
import json
import sys
import argparse
from collections import defaultdict

# Configuration
KITTY_TESTER = "./build/bin/kitty_tester"
RESULTS_FILE = "test_results.json"
MISMATCH_LOG_FILE = "mismatches.log"
SAVE_INTERVAL = 100
COMMAND_TIMEOUT = 2

# Definition of test targets
def build_vte_cmd(binary, base_cmd, key_info, flags):
    # VTE tester needs the explicit EVDEV keycode
    return [binary] + base_cmd + ['--keycode', str(key_info['keycode']), '--kitty-flags', str(flags)]

def build_far2l_cmd(binary, base_cmd, key_info, flags):
    # Far2l tester maps names internally, keycode is ignored
    return [binary] + base_cmd + ['--kitty-flags', str(flags)]

def build_alacritty_cmd(binary, base_cmd, key_info, flags):
    # Alacritty tester maps names internally based on the key name
    return [binary] + base_cmd + ['--kitty-flags', str(flags)]

TARGETS = {
    'vte': {
        'binary': './build/bin/vte_tester',
        'cmd_builder': build_vte_cmd,
        'is_fallback': lambda out: out == "[LEGACY_FALLBACK]" or out == "[EMPTY]"
    },
    'far2l': {
        'binary': './build/bin/far2l_tester',
        'cmd_builder': build_far2l_cmd,
        'is_fallback': lambda out: out == "[EMPTY]"
    },
    'alacritty': {
        'binary': './build/bin/alacritty_tester',
        'cmd_builder': build_alacritty_cmd,
        'is_fallback': lambda out: out == "[EMPTY]"
    }
}

# Standard X11/evdev keycodes for US QWERTY layout
# Used by VTE (requires evdev codes) and generic iteration
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
    # Non-English Layouts
    # 'я' corresponds to physical 'z' (keycode 52) on standard Russian layout
    'я': {'name': 'я', 'keycode': 52, 'base_key': 'z'},
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

def save_results(results, target_name):
    mismatches = [r for r in results if r['status'] == 'mismatch']

    with open(RESULTS_FILE, 'w') as f:
        json_results = []
        for r in results:
            json_r = r.copy()
            json_r['kitty_out_fmt'] = format_raw_output(json_r.pop('kitty_out'))
            json_r['target_out_fmt'] = format_raw_output(json_r.pop('target_out'))
            json_results.append(json_r)
        json.dump(json_results, f, indent=2)

    with open(MISMATCH_LOG_FILE, 'w') as f:
        f.write(f"Target: {target_name}\n")
        f.write(f"Found {len(mismatches)} mismatches.\n\n")

        max_combo_len = 0
        max_kitty_len = 0
        if mismatches:
            max_combo_len = max(len(r['combo']) for r in mismatches)
            max_kitty_len = max(len(format_raw_output(r['kitty_out'])) for r in mismatches)

        for item in mismatches:
            combo_str = item['combo'].ljust(max_combo_len)
            kitty_str = format_raw_output(item['kitty_out']).ljust(max_kitty_len)
            tgt_str = format_raw_output(item['target_out'])
            f.write(f"{combo_str} -> kitty: {kitty_str} | {target_name}: {tgt_str}\n")

def main():
    parser = argparse.ArgumentParser(description="Test and compare kitty and other terminal key encoders.")
    parser.add_argument("--debug", action="store_true", help="Enable debug output for commands.")
    parser.add_argument("--limit", type=int, default=0, help="Limit the number of test combinations to run.")
    parser.add_argument("--start-at-percent", type=int, default=0, help="Start tests from a certain percentage (0-99).")
    parser.add_argument("--target", default="vte", choices=TARGETS.keys(), help="Select the target implementation to test (default: vte).")
    args = parser.parse_args()

    target_conf = TARGETS[args.target]
    if not os.path.exists(KITTY_TESTER):
        print(f"Error: Kitty tester ({KITTY_TESTER}) not found. Run 'make' first.", file=sys.stderr)
        sys.exit(1)
    if not os.path.exists(target_conf['binary']):
        print(f"Error: Target tester ({target_conf['binary']}) not found. Run 'make' first.", file=sys.stderr)
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

    start_index = 0
    if args.start_at_percent > 0:
        if 0 < args.start_at_percent < 100:
            start_index = (total_tests * args.start_at_percent) // 100
            all_combinations = all_combinations[start_index:]
            print(f"Starting at {args.start_at_percent}%, skipping first {start_index} combinations.")

    print(f"Starting tests for target: {args.target}")
    print(f"Combinations to check: {len(all_combinations)}")

    results = []
    mismatch_count = 0
    save_counter = 0

    key_status = defaultdict(lambda: True)

    try:
        for i_offset, (key_info, mods, locks, flags) in enumerate(all_combinations):

            i = start_index + i_offset
            key_name = key_info['name']

            if i > 0 and i % 500 == 0:
                percent = ((i + 1) * 100) // total_tests
                print(f"Progress: {percent}% ({i}/{total_tests}) | Found {mismatch_count} mismatches", flush=True)

            base_cmd = ['--key', key_info['name']] + mods + locks

            # Build commands
            kitty_cmd = [KITTY_TESTER] + base_cmd + ['--kitty-flags', str(flags)]
            if 'base_key' in key_info:
                kitty_cmd.extend(['--base-key', key_info['base_key']])
            target_cmd = target_conf['cmd_builder'](target_conf['binary'], base_cmd, key_info, flags)

            # Execution
            kitty_out_raw = run_command(kitty_cmd, args.debug)
            target_out_raw = run_command(target_cmd, args.debug)

            kitty_out_str = format_raw_output(kitty_out_raw)
            target_out_str = format_raw_output(target_out_raw)

            status = 'error'

            if "[ERROR:" in kitty_out_str or "[ERROR:" in target_out_str:
                status = 'error'
            elif kitty_out_str == "[EMPTY]":
                status = 'skipped_kitty_empty'
            elif target_conf['is_fallback'](target_out_str):
                status = 'skipped_target_fallback'
            elif kitty_out_str == target_out_str:
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
                'target_out': target_out_raw,
            }
            results.append(test_case)

            if i > 0 and i % (SAVE_INTERVAL * 20) == 0:
                save_counter += 1
                print(f" [Save #{save_counter}] Saving intermediate results...")
                save_results(results, args.target)

    except KeyboardInterrupt:
        print("\nTest interrupted by user. Saving current results.")
    finally:
        sys.stdout.write("\n")
        print("Saving final results...")
        save_results(results, args.target)

        matches = len([r for r in results if r['status'] == 'match'])
        mismatches = len([r for r in results if r['status'] == 'mismatch'])
        errors = len([r for r in results if r['status'] == 'error'])

        skipped_kitty = len([r for r in results if r['status'] == 'skipped_kitty_empty'])
        skipped_target = len([r for r in results if r['status'] == 'skipped_target_fallback'])
        skipped_total = skipped_kitty + skipped_target

        total_keys_tested = len(key_status)
        successful_keys_count = sum(1 for passed in key_status.values() if passed)

        print("\n--- Test Summary ---")
        print(f"Target: {args.target}")
        print(f"Total combinations run: {len(results)} / {total_tests}")
        print(f"  Matches: {matches}")
        print(f"  Mismatches: {mismatches}")
        print(f"  Skipped: {skipped_total}")
        print(f"    Kitty return nothing: {skipped_kitty}");
        print(f"    Target falled back to legacy generation: {skipped_target}");
        print(f"  Errors: {errors}")
        print("\n--- Output Files ---")
        if mismatches or errors:
            print(f"Mismatch details: '{MISMATCH_LOG_FILE}'")
        print(f"Raw results: '{RESULTS_FILE}'")

if __name__ == "__main__":
    main()