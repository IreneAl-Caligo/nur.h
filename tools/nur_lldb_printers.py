"""
LLDB Pretty-Printers for nur.h and nur_core.h
Enables clean, idiomatic inspection of NurResult_* and NurOption_* in LLDB:
    (lldb) frame variable res
    (NurResult_i32) res = Ok(42)
    (lldb) frame variable opt
    (NurOption_i32) opt = Some(100)
    (lldb) frame variable none_opt
    (NurOption_i32) none_opt = None
"""

import sys

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

def nur_result_summary(valobj, internal_dict):
    """Type summary for NurResult_* types."""
    try:
        ok_member = valobj.GetChildMemberWithName("ok")
        if not ok_member.IsValid():
            return "NurResult(?)"

        is_ok = ok_member.GetValueAsUnsigned() != 0
        type_name = valobj.GetTypeName() or ""

        if is_ok:
            if "void" in type_name:
                return "Ok(())"
            u_member = valobj.GetChildMemberWithName("u")
            if not u_member.IsValid():
                return "Ok(?)"
            val_member = u_member.GetChildMemberWithName("value")
            val_str = val_member.GetValue() or val_member.GetSummary() or ""
            return f"Ok({val_str})"
        else:
            u_member = valobj.GetChildMemberWithName("u")
            err_member = u_member.GetChildMemberWithName("error") if u_member.IsValid() else valobj.GetChildMemberWithName("error")
            if not err_member.IsValid():
                return "Err(?)"

            code_member = err_member.GetChildMemberWithName("code")
            code_str = format_error_code(code_member.GetValueAsSigned() if code_member.IsValid() else 0)

            msg_member = err_member.GetChildMemberWithName("message")
            msg_str = msg_member.GetSummary() if msg_member.IsValid() and msg_member.GetValueAsUnsigned() != 0 else ""
            if not msg_str:
                inline_member = err_member.GetChildMemberWithName("inline_msg")
                if inline_member.IsValid():
                    msg_str = inline_member.GetSummary() or ""

            msg_info = f": {msg_str}" if msg_str else ""
            return f"Err({code_str}{msg_info})"
    except Exception as e:
        return f"NurResult<{e}>"

def nur_option_summary(valobj, internal_dict):
    """Type summary for NurOption_* types."""
    try:
        some_member = valobj.GetChildMemberWithName("some")
        if not some_member.IsValid():
            return "NurOption(?)"

        is_some = some_member.GetValueAsUnsigned() != 0
        if is_some:
            val_member = valobj.GetChildMemberWithName("value")
            val_str = val_member.GetValue() or val_member.GetSummary() or ""
            return f"Some({val_str})"
        else:
            return "None"
    except Exception as e:
        return f"NurOption<{e}>"

def nur_error_summary(valobj, internal_dict):
    """Type summary for nur_error_t structures."""
    try:
        code_member = valobj.GetChildMemberWithName("code")
        code_str = format_error_code(code_member.GetValueAsSigned() if code_member.IsValid() else 0)

        msg_member = valobj.GetChildMemberWithName("message")
        msg_str = msg_member.GetSummary() if msg_member.IsValid() and msg_member.GetValueAsUnsigned() != 0 else ""
        if not msg_str:
            inline_member = valobj.GetChildMemberWithName("inline_msg")
            if inline_member.IsValid():
                msg_str = inline_member.GetSummary() or ""

        msg_info = f', {msg_str}' if msg_str else ""
        return f"Error({code_str}{msg_info})"
    except Exception as e:
        return f"Error<{e}>"

def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand('type summary add -x "^NurResult_" -F nur_lldb_printers.nur_result_summary')
    debugger.HandleCommand('type summary add "NurResult_void" -F nur_lldb_printers.nur_result_summary')
    debugger.HandleCommand('type summary add -x "^NurOption_" -F nur_lldb_printers.nur_option_summary')
    debugger.HandleCommand('type summary add "nur_error_t" -F nur_lldb_printers.nur_error_summary')
    print("[nur.h] LLDB type summaries loaded successfully.")
