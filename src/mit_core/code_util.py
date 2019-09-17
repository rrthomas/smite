'''
Code generation utilities.

Copyright (c) 2019 Mit authors

The package is distributed under the MIT/X11 License.

THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
RISK.
'''

import re
import textwrap


class Code:
    '''
    Collect generated code, and indent it correctly.
    Represents a list of code fragments, each of which is a str or a Code.
    The latter will be indented on output.
    '''
    def __init__(self, *args):
        self.buffer = []
        for arg in args:
            self.append(arg)

    INDENT = '    '

    def __repr__(self):
        '''
        Returns a valid Python expression.
        '''
        return 'Code(\n{}\n)'.format(textwrap.indent(
            ',\n'.join(repr(x) for x in self.buffer),
            self.INDENT,
        ))

    def __str__(self):
        '''
        Returns this Code as a str.
        '''
        return '\n'.join(
            textwrap.indent(str(x), self.INDENT)
            if isinstance(x, Code) else str(x)
            for x in self.buffer
        )

    def append(self, str_or_code):
        assert isinstance(str_or_code, (Code, str))
        if isinstance(str_or_code, str):
            str_or_code = textwrap.dedent(str_or_code.rstrip())
        self.buffer.append(str_or_code)

    def extend(self, code):
        assert isinstance(code, Code)
        self.buffer.extend(code.buffer)

    def format(self, *args, **kwargs):
        return Code(*(x.format(*args, **kwargs) for x in self.buffer))


def disable_warnings(warnings, code):
    '''
    Returns `code` wrapped in "#pragmas" to suppress the given list
    `warnings` of warning flags.

     - warnings - list of str.
     - code - a Code.
    '''
    assert isinstance(code, Code)
    pragmaed_code = Code()
    pragmaed_code.append('#pragma GCC diagnostic push')
    for w in warnings:
        pragmaed_code.append('#pragma GCC diagnostic ignored "{}"'.format(w))
    pragmaed_code.extend(code)
    pragmaed_code.append('#pragma GCC diagnostic pop')
    return pragmaed_code

def c_symbol(python_symbol):
    '''
    Convert "MyIdentifier" to "MIT_MY_IDENTIFIER".
    The "MIT_" prefix is only added if not already present.
    '''
    c_symbol = re.sub('(?<!^)([A-Z])', '_\\1', python_symbol).upper()
    if not re.match('^MIT', c_symbol):
        c_symbol = 'MIT_{}'.format(c_symbol)
    return c_symbol

def enum_to_c(enum, value_extractor=lambda x: x.value):
    '''
    Convert an Enum to C code; returns a Code.

     - enum - an Enum subclass.

    Values must be int.
    '''
    prefix = c_symbol(re.sub('Code$', '', enum.__name__))
    code = Code("/* ", enum.__doc__, " */")
    for member in enum:
        code.append('{prefix}_{name} = {value},'.format(
            prefix=prefix,
            name=member.name,
            value=value_extractor(member),
        ))
    return Code(
        'enum {',
        code,
        '};',
    )

def enum_to_python(enum, value_extractor=lambda x: "{:#x}".format(x.value)):
    '''
    Convert an Enum to Python code; return a Code.
     - enum - an Enum subclass.
     - value_extractor - a function that takes an element `x` of `enum` and
       return an str. Defaults to `x.value` formatted as hex.
    '''
    class_name = enum.__name__
    code = Code("'''", enum.__doc__, "'''")
    for member in enum:
        code.append('{name} = {value}'.format(
            name=member.name,
            value=value_extractor(member),
        ))
    return Code(
        '@unique',
        'class {}(IntEnum):'.format(class_name),
        code,
    )

def unrestrict(type):
    '''Return C type without any 'restrict'.'''
    return re.sub('restrict', '', type).strip()
