#!/usr/bin/env python3
"""
wrap_legacy_c.py - Automated Migration Helper for nur.h

Scans legacy C functions and generates safe NurResult wrappers and declarations.

Usage:
    python scripts/wrap_legacy_c.py --signature "int parse_port(const char *s, int *out)"
    python scripts/wrap_legacy_c.py --scan file.c
"""

import sys
import re
import argparse

def generate_result_type(val_type, suffix):
    return f"NUR_RESULT_TYPE({val_type}, {suffix});"

def transform_out_pointer_signature(func_name, return_type, args):
    """
    Transforms: int func(args..., Type *out) -> NurResult_<suffix> func(args...)
    """
    if not args:
        return None
    
    last_arg = args[-1].strip()
    match = re.match(r"^(.*?)\s*\*\s*([a-zA-Z0-9_]+)$", last_arg)
    if not match:
        return None
    
    out_type = match.group(1).strip()
    out_name = match.group(2).strip()
    remaining_args = ", ".join(arg.strip() for arg in args[:-1]) or "void"
    
    suffix = re.sub(r"[^a-zA-Z0-9_]", "_", out_type).strip("_")
    if suffix in ("int", "int32_t"):
        suffix = "i32"
    elif suffix in ("long", "int64_t"):
        suffix = "i64"
    elif suffix in ("size_t", "uint64_t"):
        suffix = "usize"

    wrapper = f"""/* 1. Declare result type at file scope: */
{generate_result_type(out_type, suffix)}

/* 2. Modern NurResult function signature: */
NUR_NODISCARD NurResult_{suffix} {func_name}_safe({remaining_args}) {{
    {out_type} _val;
    int _rc = {func_name}({", ".join([arg.split()[-1].replace("*", "") for arg in args[:-1]] + ["&_val"])});
    if (_rc != 0) {{
        return NUR_ERR({suffix}, NUR_ERROR(NUR_E_SYSTEM, "{func_name} failed"));
    }}
    return NUR_OK({suffix}, _val);
}}
"""
    return wrapper

def transform_null_pointer_signature(func_name, return_type, args):
    """
    Transforms: Type *func(...) -> NurResult_ptr func(...)
    """
    base_type = return_type.strip()
    remaining_args = ", ".join(arg.strip() for arg in args) or "void"
    
    clean_type = base_type.replace("*", "").strip()
    suffix = f"{clean_type}_ptr" if not base_type.endswith("*") else "ptr"
    if base_type == "char *":
        suffix = "cstr"

    wrapper = f"""/* Modern NurResult pointer wrapper: */
NUR_NODISCARD NurResult_{suffix} {func_name}_safe({remaining_args}) {{
    {base_type} _ptr = {func_name}({", ".join([arg.split()[-1].replace("*", "") for arg in args if arg.strip() != "void"])});
    if (_ptr == NULL) {{
        return NUR_ERR({suffix}, NUR_ERROR_ERRNO("{func_name} returned NULL"));
    }}
    return NUR_OK({suffix}, _ptr);
}}
"""
    return wrapper

def process_signature(sig):
    sig = sig.strip().rstrip(";")
    pattern = r"^\s*([a-zA-Z0-9_\s\*]+?)\s*(\b[a-zA-Z0-9_]+)\s*\((.*?)\)$"
    match = re.match(pattern, sig)
    if not match:
        print(f"[-] Could not parse function signature: {sig}", file=sys.stderr)
        return

    raw_ret = match.group(1).strip()
    func_name = match.group(2).strip()
    raw_args = match.group(3).strip()
    args = [a.strip() for a in raw_args.split(",")] if raw_args and raw_args != "void" else []

    # If the return type has pointers, normalize it
    ret_type = " ".join(raw_ret.split())

    print(f"\n=======================================================")
    print(f"  Migration Recipe for: {func_name}()")
    print(f"=======================================================\n")

    if ret_type in ("int", "int32_t", "long") and args and "*" in args[-1]:
        print(transform_out_pointer_signature(func_name, ret_type, args))
    elif "*" in ret_type:
        print(transform_null_pointer_signature(func_name, ret_type, args))
    else:
        print(f"/* Void result validation pattern: */\nNUR_NODISCARD NurResult_void {func_name}_safe({raw_args}) {{\n    int _rc = {func_name}(...);\n    if (_rc != 0) return nur_err_void(NUR_ERROR(NUR_E_SYSTEM, \"{func_name} failed\"));\n    return nur_ok_void();\n}}\n")

def scan_file(filepath):
    print(f"Scanning {filepath} for legacy C function patterns...")
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    patterns = [
        (r"\bint\s+([a-zA-Z0-9_]+)\s*\(([^)]*\*\s*[a-zA-Z0-9_]+)\s*\)", "Output-pointer based function"),
        (r"\b([a-zA-Z0-9_]+\s*\*)\s*([a-zA-Z0-9_]+)\s*\(([^)]*)\)", "NULL-pointer returning function"),
    ]

    for regex, desc in patterns:
        for m in re.finditer(regex, content):
            matched_sig = m.group(0).replace("\n", " ")
            print(f"[*] Detected {desc}: {matched_sig}")

def main():
    parser = argparse.ArgumentParser(description="nur.h Legacy C Migration Assistant")
    parser.add_argument("--signature", "-s", help="C function signature to wrap")
    parser.add_argument("--scan", "-f", help="Scan C source file for candidates")

    args = parser.parse_args()
    if args.signature:
        process_signature(args.signature)
    elif args.scan:
        scan_file(args.scan)
    else:
        print("Demo: Generating wrappers for typical legacy C patterns:\n")
        process_signature("int parse_port(const char *s, int *out)")
        process_signature("FILE *open_file(const char *path, const char *mode)")
        process_signature("int validate_config(const void *cfg)")

if __name__ == "__main__":
    main()
