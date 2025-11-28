import pytest # type: ignore
from binding import GenerateBindings
from models import Binding
from tests.utils import AssertBindingResult


def test_generate_simple_binding_function():
    binding = Binding(
        file="",
        template="autogen/templates/binding/function.j2",
    )

    content = GenerateBindings(binding, testContent="""
void __attribute__((annotate("binding"))) simple_function(int a, float b);
""")

    expected = """
    module.def("simple_function", &simple_function, "No docstring");
"""

    AssertBindingResult(expected, content)

def test_generate_function_with_comments():
    binding = Binding(
        file="",
        template="autogen/templates/binding/function.j2",
    )

    content = GenerateBindings(binding, testContent="""
/**
 * @brief This is a simple function.
 */
void __attribute__((annotate("binding"))) simple_function(int a, float b);
""")
    
    expected = """
    module.def("simple_function", &simple_function, "This is a simple function.");
"""

    AssertBindingResult(expected, content)

def test_generate_function_without_binding_annotation():
    binding = Binding(
        file="",
        template="autogen/templates/binding/function.j2",
    )

    content = GenerateBindings(binding, testContent="""
void simple_function(int a, float b);
""")
    
    expected = """"""

    AssertBindingResult(expected, content)

def test_generate_multiple_functions():
    binding = Binding(
        file="",
        template="autogen/templates/binding/function.j2",
    )

    content = GenerateBindings(binding, testContent="""
void __attribute__((annotate("binding"))) function_one(int a);
float __attribute__((annotate("binding"))) function_two(float b, double c);
""")
    
    expected = """
    module.def("function_one", &function_one, "No docstring");
    module.def("function_two", &function_two, "No docstring");
"""

    AssertBindingResult(expected, content)