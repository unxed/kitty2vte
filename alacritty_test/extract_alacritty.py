#!/usr/bin/env python3
import sys
import os

def extract_function(lines, start_str):
    captured_lines = []
    capturing = False
    brace_count = 0
    found_opening_brace = False
    
    for line in lines:
        if not capturing:
            if start_str in line:
                capturing = True
                # Normalize indentation for the function definition line
                # We assume it was indented by 4 spaces in the impl block
                clean_line = line.lstrip()
                if not clean_line.startswith("pub"):
                     clean_line = "pub " + clean_line
                
                captured_lines.append(clean_line)
                
                brace_count += line.count('{')
                brace_count -= line.count('}')
                if '{' in line:
                    found_opening_brace = True
        else:
            # Unindent logic: remove 4 spaces if they exist (standard Rust indent)
            if line.startswith("    "):
                clean_line = line[4:]
            else:
                clean_line = line
            
            captured_lines.append(clean_line)
            
            brace_count += line.count('{')
            brace_count -= line.count('}')
            
            if '{' in line:
                found_opening_brace = True
            
            # Only break if we have started the block AND matched the closing brace
            if found_opening_brace and brace_count <= 0:
                break
                
    return captured_lines

def extract(source_path, dest_path):
    if not os.path.exists(source_path):
        print(f"Error: Source file '{source_path}' not found.", file=sys.stderr)
        sys.exit(1)

    with open(source_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    output = []
    output.append("// Extracted logic from Alacritty's keyboard.rs\n")
    output.append("use std::borrow::Cow;\n")
    output.append("use crate::alacritty_mocks::*;\n\n")

    # 1. Extract should_build_sequence
    print(f"[*] Extracting should_build_sequence...")
    sbs_lines = extract_function(lines, "fn should_build_sequence")
    if not sbs_lines:
        print("Error: Could not find should_build_sequence", file=sys.stderr)
        sys.exit(1)
    output.extend(sbs_lines)
    output.append("\n")

    # 2. Extract build_sequence and everything following it
    print(f"[*] Extracting build_sequence and helpers...")
    capturing = False
    for line in lines:
        if "fn build_sequence" in line:
            capturing = True
        
        if capturing:
            if line.strip().startswith("use "):
                continue
            
            # Fix: Remove bitflags crate namespace
            line = line.replace("bitflags::bitflags!", "bitflags!")
            
            output.append(line)

    with open(dest_path, 'w', encoding='utf-8') as f:
        f.writelines(output)
    
    print(f"[*] Extracted Alacritty logic to '{dest_path}'")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        source = 'source/keyboard.rs'
    else:
        source = sys.argv[1]
    
    dest = 'alacritty_test/alacritty_extracted.rs'
    extract(source, dest)