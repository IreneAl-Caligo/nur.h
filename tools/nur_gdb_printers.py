"""
GDB Pretty-Printers for nur.h and nur_core.h
Enables clean, idiomatic inspection of NurResult_* and NurOption_* in GDB:
    (gdb) print res
    $1 = Ok(42)
    (gdb) print err_res
    $2 = Err(NUR_E_OUT_OF_RANGE: "port out of range", at server.c:45)
    (gdb) print opt
    $3 = Some(100)
    (gdb) print none_opt
    $4 = None
"""

import re
import sys

# Systematic Error Code Names mapping
ERROR_CODE_NAMES = {
    0: "NUR_E_NONE",
    100: "NUR_E_INVALID_ARGUMENT",
    101: "NUR_E_NULL_POINTER",
    102: "NUR_E_OUT_OF_RANGE",
    103: "NUR_E_TYPE_MISMATCH",
    104: "NUR_E_BUFFER_OVERFLOW",
    105: "NUR_E_DIVIDE_BY_ZERO",
    106: "NUR_E_MATH_DOMAIN",
    107: "NUR_E_INTEGER_OVERFLOW",
    200: "NUR_E_NOT_FOUND",
    201: "NUR_E_ALREADY_EXISTS",
    202: "NUR_E_BUSY",
    203: "NUR_E_TIMEOUT",
    204: "NUR_E_RESOURCE_EXHAUSTED",
    300: "NUR_E_IO_ERROR",
    301: "NUR_E_FILE_NOT_FOUND",
    302: "NUR_E_PERMISSION_DENIED",
    303: "NUR_E_END_OF_FILE",
    304: "NUR_E_DISK_FULL",
    305: "NUR_E_BROKEN_PIPE",
    400: "NUR_E_CONNECTION_REFUSED",
    401: "NUR_E_CONNECTION_RESET",
    402: "NUR_E_HOST_UNREACHABLE",
    403: "NUR_E_NETWORK_DOWN",
    404: "NUR_E_ADDRESS_IN_USE",
    500: "NUR_E_INVALID_STATE",
    501: "NUR_E_OPERATION_FAILED",
    502: "NUR_E_ABORTED",
    503: "NUR_E_DEADLOCK",
    600: "NUR_E_PARSE_ERROR",
    601: "NUR_E_SYNTAX_ERROR",
    602: "NUR_E_VALIDATION_FAILED",
    603: "NUR_E_ENCODING_ERROR",
    700: "NUR_E_ACCESS_DENIED",
    701: "NUR_E_AUTHENTICATION_FAILED",
    702: "NUR_E_CRYPTO_ERROR",
    800: "NUR_E_UNIMPLEMENTED",
    801: "NUR_E_NOT_SUPPORTED",
    802: "NUR_E_OBSOLETE",
    900: "NUR_E_UNKNOWN",
    901: "NUR_E_INTERNAL",
    902: "NUR_E_EXTERNAL",
    1000: "NUR_E_USER_BASE",
}

def format_error_code(code_val):
    try:
        c = int(code_val)
        return ERROR_CODE_NAMES.get(c, f"NUR_ERROR({c})")
    except Exception:
        return str(code_val)

def extract_c_string(val_ptr):
    if not val_ptr:
        return ""
    try:
        if hasattr(val_ptr, 'string'):
            return val_ptr.string()
        return str(val_ptr)
    except Exception:
        return ""

class NurResultPrinter:
    """Pretty-printer for NurResult_* and NurResult_void."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        try:
            ok = bool(self.val['ok'])
            type_name = str(self.val.type)

            if ok:
                if "void" in type_name:
                    return "Ok(())"
                # Extract value
                val = self.val['u']['value']
                return f"Ok({val})"
            else:
                # Extract error
                err = self.val['u']['error']
                code = format_error_code(err['code'])

                msg = ""
                # Check .message borrowed pointer first
                if err['message']:
                    msg = extract_c_string(err['message'])
                # Fallback to inline_msg if message was empty
                if not msg and 'inline_msg' in [f.name for f in err.type.fields()]:
                    msg = extract_c_string(err['inline_msg'])

                file_str = extract_c_string(err['file']) if 'file' in [f.name for f in err.type.fields()] else ""
                line_val = int(err['line']) if 'line' in [f.name for f in err.type.fields()] else 0

                loc_info = f", at {file_str}:{line_val}" if file_str and line_val > 0 else ""
                msg_info = f': "{msg}"' if msg else ""

                return f"Err({code}{msg_info}{loc_info})"
        except Exception as e:
            return f"NurResult<{e}>"


class NurOptionPrinter:
    """Pretty-printer for NurOption_* types."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        try:
            some = bool(self.val['some'])
            if some:
                val = self.val['value']
                return f"Some({val})"
            else:
                return "None"
        except Exception as e:
            return f"NurOption<{e}>"


class NurErrorPrinter:
    """Pretty-printer for nur_error_t structures."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        try:
            code = format_error_code(self.val['code'])
            msg = extract_c_string(self.val['message']) if self.val['message'] else ""
            if not msg and 'inline_msg' in [f.name for f in self.val.type.fields()]:
                msg = extract_c_string(self.val['inline_msg'])

            file_str = extract_c_string(self.val['file']) if 'file' in [f.name for f in self.val.type.fields()] else ""
            line_val = int(self.val['line']) if 'line' in [f.name for f in self.val.type.fields()] else 0

            loc = f" at {file_str}:{line_val}" if file_str and line_val > 0 else ""
            msg_str = f', "{msg}"' if msg else ""
            return f"Error({code}{msg_str}{loc})"
        except Exception as e:
            return f"Error<{e}>"


def build_pretty_printer():
    import gdb.printing
    pp = gdb.printing.RegexpCollectionPrettyPrinter("nur")
    pp.add_printer('NurResult', r'^NurResult_', NurResultPrinter)
    pp.add_printer('NurResult_void', r'^NurResult_void$', NurResultPrinter)
    pp.add_printer('NurOption', r'^NurOption_', NurOptionPrinter)
    pp.add_printer('nur_error_t', r'^nur_error_t$', NurErrorPrinter)
    return pp


def register_nur_printers(objfile=None):
    try:
        import gdb.printing
        gdb.printing.register_pretty_printer(objfile, build_pretty_printer(), replace=True)
        print("[nur.h] GDB pretty-printers registered successfully.")
    except Exception as e:
        print(f"[nur.h] GDB pretty-printer registration error: {e}", file=sys.stderr)


# Auto-register when imported under GDB
if "gdb" in sys.modules:
    register_nur_printers()
