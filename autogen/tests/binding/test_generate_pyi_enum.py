import pytest  # type: ignore
from binding import GenerateBindings
from models import Binding
from tests.utils import AssertBindingResult


def test_generate_simple_enum():
    binding: Binding = Binding(
        file="",
        template="autogen/templates/pyi/enum.j2",
        output="",
    )

    result = GenerateBindings(
        binding,
        testContent="""
enum __attribute__((annotate("binding"))) Color {
    RED,
};
""",
    )

    expect = """
class Color(Enum):
    RED = 0
"""

    AssertBindingResult(expect, result)


def test_generate_empty_enum():
    binding: Binding = Binding(
        file="",
        template="autogen/templates/pyi/enum.j2",
        output="",
    )

    result = GenerateBindings(
        binding,
        testContent="""
enum __attribute__((annotate("binding"))) Empty {
};
""",
    )

    expect = """
class Empty(Enum):
    pass
"""

    AssertBindingResult(expect, result)


def test_generate_enum_with_comments():
    binding: Binding = Binding(
        file="",
        template="autogen/templates/pyi/enum.j2",
        output="",
    )

    result = GenerateBindings(
        binding,
        testContent="""
/**
 * @brief This is an enum with comments.
 */
enum __attribute__((annotate("binding"))) Color {
    RED, ///< Red color
    GREEN,
    BLUE = 3 ///< Blue color
};
""",
    )

    expect = """
class Color(Enum):
    \"\"\"
    This is an enum with comments.
    \"\"\"
    RED = 0
    \"\"\"
    Red color
    \"\"\"
    GREEN = 1
    BLUE = 3
    \"\"\"
    Blue color
    \"\"\"
"""

    AssertBindingResult(expect, result)


def test_generate_enum_with_hidden_annotations():
    binding: Binding = Binding(
        file="",
        template="autogen/templates/pyi/enum.j2",
        output="",
    )

    result = GenerateBindings(
        binding,
        testContent="""
enum __attribute__((annotate("binding"))) Color {
    RED,
    GREEN,
    BLUE,
    COUNT __attribute__((annotate("hidden"))),
        """,
    )

    expect = """
class Color(Enum):
    RED = 0
    GREEN = 1
    BLUE = 2
"""

    AssertBindingResult(expect, result)


def test_generate_enum_with_non_binding_annotation():
    binding: Binding = Binding(
        file="",
        template="autogen/templates/pyi/enum.j2",
        output="",
    )

    result = GenerateBindings(
        binding,
        testContent="""
enum Color {
    RED,
    GREEN,
    BLUE,
};
""",
    )

    AssertBindingResult("\n", result)
