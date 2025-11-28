import pytest  # type: ignore
from binding import GenerateBindings
from models import Binding
from tests.utils import AssertBindingResult


def test_generate_simple_typedef():
    binding: Binding = Binding(
        template="autogen/templates/pyi/typedef.j2",
    )

    result = GenerateBindings(
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
