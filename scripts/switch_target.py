#!/usr/bin/env python3

import argparse
import os
import sys

# Define environment names
QEMU_ENV = "qemu-lm3s6965evb"
HARDWARE_ENV = "weact_g431cb_hardware"
PIO_INI = "platformio.ini"

def switch_pio_target(target, dry_run=False):
    """
    Switches the default PlatformIO target environment in platformio.ini.

    Args:
        target (str): The target environment name ('qemu' or 'hardware').
        dry_run (bool): If True, print changes without modifying the file.
    """
    target_env = ""
    if target == "qemu":
        target_env = QEMU_ENV
    elif target == "hardware":
        target_env = HARDWARE_ENV
    else:
        print(f"Error: Invalid target '{target}'. Use 'qemu' or 'hardware'.", file=sys.stderr)
        return False

    pio_ini_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", PIO_INI)

    if not os.path.exists(pio_ini_path):
        print(f"Error: '{PIO_INI}' not found at '{pio_ini_path}'. Make sure you are in the project root or the script is in ./scripts.", file=sys.stderr)
        return False

    print(f"Attempting to switch default PlatformIO target to '{target_env}' in '{PIO_INI}'...")

    lines = []
    in_platformio_section = False
    default_envs_found = False
    modified_lines = []

    with open(pio_ini_path, 'r') as f:
        lines = f.readlines()

    for line in lines:
        stripped_line = line.strip()
        if stripped_line == "[platformio]":
            in_platformio_section = True
            modified_lines.append(line)
        elif stripped_line.startswith("[env:"):
            # Reached another section, exit platformio section context
            in_platformio_section = False
            modified_lines.append(line)
        elif in_platformio_section and stripped_line.startswith("default_envs ="):
            # Found the default_envs line, modify it
            new_line = f"default_envs = {target_env}"
            modified_lines.append(new_line)
            default_envs_found = True
            print(f"  Found and modified line: '{line.strip()}' -> '{new_line.strip()}'")
        else:
            # Keep other lines as they are
            modified_lines.append(line)

    if not default_envs_found:
        print(f"Error: 'default_envs =' line not found within the '[platformio]' section in '{PIO_INI}'.", file=sys.stderr)
        print("Please ensure platformio.ini has a [platformio] section and a default_envs line.", file=sys.stderr)
        return False

    if dry_run:
        print("-- Dry Run Output (Not writing to file) ---")
        print("".join(modified_lines))
        print("--- End Dry Run ---")
        print(f"Dry run complete. Default target *would have been* switched to '{target_env}'.")
    else:
        try:
            with open(pio_ini_path, 'w') as f:
                f.writelines(modified_lines)
            print(f"Successfully switched default target to '{target_env}' in '{PIO_INI}'.")
            print("Run 'pio run' to build for the new target.")
            return True
        except IOError as e:
            print(f"Error writing to '{pio_ini_path}': {e}", file=sys.stderr)
            return False

def main():
    parser = argparse.ArgumentParser(description="Switch default PlatformIO build target (QEMU or Hardware).")
    parser.add_argument("target", choices=["qemu", "hardware"], help="The target environment to set as default ('qemu' or 'hardware').")
    parser.add_argument("--dry-run", action="store_true", help="Perform a dry run, showing changes without modifying the file.")

    args = parser.parse_args()

    success = switch_pio_target(args.target, args.dry_run)

    if not success:
        sys.exit(1)
    else:
        sys.exit(0)

if __name__ == "__main__":
    main()