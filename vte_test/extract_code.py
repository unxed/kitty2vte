#!/usr/bin/env python3
import sys
import os

def extract_function_body(source_path, dest_path, signature, stop_marker):
    if not os.path.exists(source_path):
        print(f"Error: Source file '{source_path}' not found.", file=sys.stderr)
        sys.exit(1)

    with open(source_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    body = []
    capturing = False

    iter_lines = iter(lines)
    for line in iter_lines:
        if signature in line:
            while '{' not in line:
                line = next(iter_lines)
            capturing = True
            break

    if not capturing:
        print(f"Error: Function start for '{signature}' not found.", file=sys.stderr)
        sys.exit(1)

    body.append('fprintf(stderr, "[DEBUG] Entering extracted code block.\\n");\n')

    brace_level = 1
    for line in iter_lines:
        if stop_marker in line:
            body.append('fprintf(stderr, "[DEBUG] Hit legacy_fallback, breaking.\\n");\n')
            break

        if "auto skipped_param2 = false;" in line:
            body.append('fprintf(stderr, "[DEBUG] Modifiers value before formatting: %u\\n", modifiers);\n')
            body.append('fprintf(stderr, "[DEBUG] Event type value before formatting: %d\\n", event_type);\n')

        processed_line = line.replace('goto legacy_fallback;', 'return false;')
        body.append(processed_line)

        brace_level += line.count('{')
        brace_level -= line.count('}')
        if brace_level <= 0:
             break

    if not body:
        print(f"Error: Function body for '{signature}' is empty.", file=sys.stderr)
        sys.exit(1)

    with open(dest_path, 'w', encoding='utf-8') as f:
        f.writelines(body)

    print(f"[*] Extracted function body to '{dest_path}'")

if __name__ == "__main__":
    extract_function_body(
        source_path='source/vte.cc',
        dest_path='vte_test/vte_key_press_body.inc',
        signature='Terminal::widget_key_press',
        stop_marker='legacy_fallback:'
    )