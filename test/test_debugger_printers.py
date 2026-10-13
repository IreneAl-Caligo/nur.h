"""
Unit test suite verifying GDB & LLDB pretty-printer logic using mocks.
"""

import sys
import unittest
from pathlib import Path

# Add tools to sys.path
sys.path.insert(0, str(Path(__file__).parent.parent / "tools"))

import nur_gdb_printers
import nur_lldb_printers

class MockField:
    def __init__(self, name):
        self.name = name

class MockType:
    def __init__(self, name, fields=None):
        self._name = name
        self._fields = [MockField(f) for f in (fields or [])]

    def __str__(self):
        return self._name

    def fields(self):
        return self._fields

class MockGdbValue:
    def __init__(self, val_dict, type_name="NurResult_i32", fields=None):
        self.val_dict = val_dict
        self.type = MockType(type_name, fields or list(val_dict.keys()))

    def __getitem__(self, key):
        return self.val_dict[key]

class MockCString:
    def __init__(self, text):
        self.text = text

    def string(self):
        return self.text

    def __bool__(self):
        return bool(self.text)

class TestGdbPrinters(unittest.TestCase):
    def test_result_ok(self):
        val = MockGdbValue({
            'ok': True,
            'u': {'value': 42}
        }, type_name="NurResult_i32")
        printer = nur_gdb_printers.NurResultPrinter(val)
        self.assertEqual(printer.to_string(), "Ok(42)")

    def test_result_void_ok(self):
        val = MockGdbValue({
            'ok': True,
            'u': {}
        }, type_name="NurResult_void")
        printer = nur_gdb_printers.NurResultPrinter(val)
        self.assertEqual(printer.to_string(), "Ok(())")

    def test_result_err(self):
        err_fields = ['code', 'message', 'file', 'line', 'inline_msg']
        err_val = MockGdbValue({
            'code': 102,
            'message': MockCString("port out of range"),
            'file': MockCString("server.c"),
            'line': 45,
            'inline_msg': MockCString("")
        }, type_name="nur_error_t", fields=err_fields)

        val = MockGdbValue({
            'ok': False,
            'u': {'error': err_val}
        }, type_name="NurResult_i32")
        printer = nur_gdb_printers.NurResultPrinter(val)
        self.assertEqual(printer.to_string(), 'Err(NUR_E_OUT_OF_RANGE: "port out of range", at server.c:45)')

    def test_option_some(self):
        val = MockGdbValue({
            'some': True,
            'value': 100
        }, type_name="NurOption_i32")
        printer = nur_gdb_printers.NurOptionPrinter(val)
        self.assertEqual(printer.to_string(), "Some(100)")

    def test_option_none(self):
        val = MockGdbValue({
            'some': False,
            'value': 0
        }, type_name="NurOption_i32")
        printer = nur_gdb_printers.NurOptionPrinter(val)
        self.assertEqual(printer.to_string(), "None")

if __name__ == '__main__':
    unittest.main()
