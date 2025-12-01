import pytest  # type: ignore
from binding import GenerateBindings
from models import Binding
from tests.utils import AssertBindingResult


def test_generate_pyi_struct() -> None:
    binding = Binding(
        template="autogen/templates/pyi/struct.j2",
    )

    result, _ = GenerateBindings(
        binding,
        """
struct __attribute__((annotate("binding"))) Point {
    int x;
    int y;
};
    """,
    )

    expected = """
class Point:

    x: int

    y: int
"""

    AssertBindingResult(expected, result)


def test_generate_pyi_struct_no_annotate() -> None:
    binding = Binding(
        template="autogen/templates/pyi/struct.j2",
    )

    result, _ = GenerateBindings(
        binding,
        """
struct Point {
    int x;
    int y;
};
    """,
    )

    expected = """"""

    AssertBindingResult(expected, result)


def test_generate_pyi_struct_with_comment() -> None:
    binding = Binding(
        template="autogen/templates/pyi/struct.j2",
    )

    result, _ = GenerateBindings(
        binding,
        """
/// This is a 2D point structure
struct __attribute__((annotate("binding"))) Point {
    /// X coordinate
    int x;
    int y; ///< Y coordinate
};
    """,
    )

    expected = """
class Point:
    \"\"\"
    This is a 2D point structure
    \"\"\"

    x: int
    \"\"\"
    X coordinate
    \"\"\"

    y: int
    \"\"\"
    Y coordinate
    \"\"\"
"""

    AssertBindingResult(expected, result)


def test_generate_multiple_structs() -> None:
    binding = Binding(
        template="autogen/templates/pyi/struct.j2",
    )

    result, _ = GenerateBindings(
        binding,
        """
struct __attribute__((annotate("binding"))) Point {
    int x;
    int y;
};

struct __attribute__((annotate("binding"))) Rectangle {
    struct Point top_left;
    struct Point bottom_right;
};
    """,
    )

    expected = """
class Point:

    x: int

    y: int  


class Rectangle:

    top_left: Point

    bottom_right: Point
"""

    AssertBindingResult(expected, result)
