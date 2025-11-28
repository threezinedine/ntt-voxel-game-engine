import pytest  # type: ignore
from binding import GenerateBindings
from models import Binding
from tests.utils import AssertBindingResult


def test_generate_simple_typedef():
    binding: Binding = Binding(
        template="autogen/templates/pyi/typedef.j2",
    )

    result, _ = GenerateBindings(
        binding,
        testContent="""
typedef __attribute__((annotate("binding"))) unsigned int uint32_t;
typedef __attribute__((annotate("binding"))) const char* cstr_t;
typedef int unused_t;
""",
    )

    expect = """
uint32_t: TypeAlias = int
cstr_t: TypeAlias = str
"""

    AssertBindingResult(expect, result)


def test_generate_typedef_with_comment():
    binding: Binding = Binding(
        template="autogen/templates/pyi/typedef.j2",
    )

    result, _ = GenerateBindings(
        binding,
        testContent="""
/// This is a typedef for a 32-bit unsigned integer.
typedef __attribute__((annotate("binding"))) unsigned int uint32_t; 
""",
    )

    expect = """
# This is a typedef for a 32-bit unsigned integer.
uint32_t: TypeAlias = int
"""

    AssertBindingResult(expect, result)
