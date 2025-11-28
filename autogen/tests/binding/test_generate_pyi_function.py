import pytest  # type: ignore
from binding import GenerateBindings
from models import Binding
from tests.utils import AssertBindingResult


def test_generate_simple_function() -> None:
    binding = Binding(
        template="autogen/templates/pyi/function.j2",
    )

    result, _ = GenerateBindings(
        binding=binding,
        testContent="""
    void simple_function() __attribute__((annotate("binding")));
    """,
    )

    expected = """
def simple_function() -> None:
    ...
    """

    AssertBindingResult(expected, result)


def test_generate_function_with_parameters() -> None:
    binding = Binding(
        template="autogen/templates/pyi/function.j2",
    )

    result, _ = GenerateBindings(
        binding=binding,
        testContent="""
    int add(int a, int b) __attribute__((annotate("binding")));
    """,
    )

    expected = """
def add(a: int, b: int) -> int:
    ...
    """

    AssertBindingResult(expected, result)


def test_generate_function_without_annotation() -> None:
    binding = Binding(
        template="autogen/templates/pyi/function.j2",
    )

    result, _ = GenerateBindings(
        binding=binding,
        testContent="""
    void not_bound_function();
    """,
    )

    expected = """"""

    AssertBindingResult(expected, result)


def test_generate_function_with_comment() -> None:
    binding = Binding(
        template="autogen/templates/pyi/function.j2",
    )

    result, _ = GenerateBindings(
        binding=binding,
        testContent="""
    /** 
     * @brief Multiplies two floating point numbers.
     * @param x First number.
     * @param y Second number.
     * @return The product of x and y.
     */
    float multiply(float x, float y) __attribute__((annotate("binding")));
    """,
    )

    expected = """
def multiply(x: float, y: float) -> float:
    \"\"\"
    :brief Multiplies two floating point numbers.
    :param x First number.
    :param y Second number.
    :return The product of x and y.
    \"\"\"
    ...
    """

    AssertBindingResult(expected, result)


def test_generate_function_with_enum_parameter() -> None:
    binding = Binding(
        template="autogen/templates/pyi/function.j2",
    )

    result, _ = GenerateBindings(
        binding=binding,
        testContent="""
    enum __attribute__((annotate("binding"))) Color {
        RED,
        GREEN,
        BLUE
    };

    void set_color(enum Color color) __attribute__((annotate("binding")));
    """,
    )

    expected = """
def set_color(color: Color) -> None:
    ...
    """

    AssertBindingResult(expected, result)


def test_generate_non_binding_argument_function() -> None:
    binding = Binding(
        template="autogen/templates/pyi/function.j2",
    )

    result, _ = GenerateBindings(
        binding=binding,
        testContent="""
    enum Mode {
        MODE_A,
        MODE_B
    };

    /** 
     * @brief Initializes the system.
     */
    void initialize_system(int mode, enum Mode mode_enum) __attribute__((annotate("binding")));
    """,
    )

    expected = """
def initialize_system(mode: int, mode_enum: Any) -> None:
    \"\"\"
    :brief Initializes the system.
    \"\"\"
    ...
    """

    AssertBindingResult(expected, result)
