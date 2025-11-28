import pytest  # type: ignore
from binding import GenerateBindings
from models import Binding
from tests.utils import AssertBindingResult


def test_generate_pyi_struct() -> None:
    binding = Binding(
        template="autogen/templates/binding/struct.j2",
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
py::class_<Point>(module, "Point")
    .def(py::init<>())
    .def_readwrite("x", &Point::x)
    .def_readwrite("y", &Point::y)
    ;
"""

    AssertBindingResult(expected, result)


def test_generate_pyi_struct_no_annotate() -> None:
    binding = Binding(
        template="autogen/templates/binding/struct.j2",
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


def test_generate_pyi_struct_with_hidden_field() -> None:
    binding = Binding(
        template="autogen/templates/binding/struct.j2",
    )

    result, _ = GenerateBindings(
        binding,
        """
struct __attribute__((annotate("binding"))) Point {
    int x;
    int y __attribute__((annotate("hidden")));
};
        """,
    )

    expected = """
py::class_<Point>(module, "Point")
    .def(py::init<>())
    .def_readwrite("x", &Point::x)
    ;
"""

    AssertBindingResult(expected, result)


def test_generate_empty_struct() -> None:
    binding = Binding(
        template="autogen/templates/binding/struct.j2",
    )

    result, _ = GenerateBindings(
        binding,
        """
struct __attribute__((annotate("binding"))) Empty {
};
        """,
    )

    expected = """
py::class_<Empty>(module, "Empty")
    .def(py::init<>())
    ;
"""

    AssertBindingResult(expected, result)
