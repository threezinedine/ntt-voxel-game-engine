import pytest  # type: ignore
from binding import GenerateBindings
from models import Binding
from tests.utils import AssertBindingResult


def test_generate_simple_enum():
    binding: Binding = Binding(
        template="autogen/templates/binding/enum.j2",
    )

    result, _ = GenerateBindings(
        binding,
        testContent="""
enum __attribute__((annotate("binding"))) Color {
    RED,
    BLUE,
    GREEN
};
""",
    )

    expect = """
py::enum_<Color>(module, "Color")
    .value("RED", Color::RED)
    .value("BLUE", Color::BLUE)
    .value("GREEN", Color::GREEN)
    .export_values();
"""

    AssertBindingResult(expect, result)


def test_generate_empty_enum():
    binding: Binding = Binding(
        template="autogen/templates/binding/enum.j2",
    )

    result, _ = GenerateBindings(
        binding,
        testContent="""
enum __attribute__((annotate("binding"))) Empty {
};
""",
    )

    expect = """
py::enum_<Empty>(module, "Empty")
    .export_values();
"""

    AssertBindingResult(expect, result)


def test_generate_enum_with_hidden_constants():
    binding: Binding = Binding(
        template="autogen/templates/binding/enum.j2",
    )

    result, _ = GenerateBindings(
        binding,
        testContent="""
enum __attribute__((annotate("binding"))) Status {
    OK,
    ERROR,
    HIDDEN __attribute__((annotate("hidden"))),
};
""",
    )

    expect = """
py::enum_<Status>(module, "Status")
    .value("OK", Status::OK)
    .value("ERROR", Status::ERROR)
    .export_values();
"""

    AssertBindingResult(expect, result)
