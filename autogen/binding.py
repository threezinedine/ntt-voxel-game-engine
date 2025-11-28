import os
from pathlib import Path
from utils import SYSTEM
from analyze import Parser
from models import Binding
from jinja2 import Environment
from jinja2 import FileSystemLoader
from utils import IsFileModified, logger
from analyze.py_function import PyFunction, PyObject

TAB_STRING = "    "
parser: Parser | None = None


def _CTypeConvert(cType: str) -> str:
    """
    Convert a C type to a Python type.

    Arguments
    ---------
    cType : str
        The C type to convert.

    Returns
    -------
    str
        The converted Python type.
    """

    cType = cType.strip()

    if cType.startswith("enum "):
        cType = cType[5:].strip()

    if cType.startswith("struct "):
        cType = cType[7:].strip()

    global parser
    assert parser is not None, "Parser is not initialized."

    if cType in [
        "unsigned int",
        "uint32_t",
        "uint16_t",
        "uint8_t",
        "int",
        "short",
        "long",
        "unsigned short",
        "unsigned long",
        "int32_t",
        "int16_t",
        "int8_t",
        "long long",
        "unsigned long long",
        "int64_t",
        "uint64_t",
        "unsigned char",
        "char",
        "signed char",
    ]:
        return "int"
    elif cType in ["float", "double"]:
        return "float"
    elif cType in [
        "const char *",
        "char *",
    ]:
        return "str"
    elif cType == "void":
        return "None"
    elif cType in parser.AllCustomTypes:
        return cType
    else:
        logger.warning(f'Unknown C type "{cType}", mapping to "Any".')
        return "Any"


def _GetFunctionParameters(function: PyFunction) -> str:
    """
    Get the function parameters as a string.

    Arguments
    ---------
    function : PyFunction
        The function to get the parameters from.

    Returns
    -------
    str
        The function parameters as a string.
    """

    params: list[str] = []
    for argument in function.arguments:
        paramType = _CTypeConvert(argument.type)
        params.append(f"{argument.name}: {paramType}")

    return ", ".join(params)


def _ConvertRawCCommentToPythonDocstring(pyObject: PyObject) -> str:
    """
    Convert a raw C comment to a Python docstring.

    Arguments
    ---------
    comment : str
        The raw C comment.

    Returns
    -------
    str
        The converted Python docstring.
    """

    assert pyObject is not None, "pyObject cannot be None."

    if pyObject.rawComent is None:
        return f"{TAB_STRING}"

    lines = pyObject.rawComent.splitlines()
    transferredLines: list[str] = []

    for line in lines:
        lineCharsCount = len(line)
        index = 0
        while index < lineCharsCount and line[index] in [
            " ",
            "/",
            "*",
            "<",
        ]:
            index += 1

        if index == lineCharsCount:
            continue

        cutLine = line[index:].strip()
        cutLine = cutLine.replace("@", ":")
        transferredLines.append(cutLine)

    return (
        f'{TAB_STRING}"""\n{TAB_STRING}'
        + f"\n{TAB_STRING}".join(transferredLines)
        + f'\n{TAB_STRING}"""\n{TAB_STRING}'
    )


def _ConvertRawCCommentToPythonComment(pyObject: PyObject) -> str:
    """
    Convert a raw C comment to a Python comment.

    Arguments
    ---------
    comment : str
        The raw C comment.

    Returns
    -------
    str
        The converted Python comment.
    """

    assert pyObject is not None, "pyObject cannot be None."

    if pyObject.rawComent is None:
        return ""

    lines = pyObject.rawComent.splitlines()
    transferredLines: list[str] = []

    for line in lines:
        lineCharsCount = len(line)
        index = 0
        while index < lineCharsCount and line[index] in [
            " ",
            "/",
            "*",
            "<",
        ]:
            index += 1

        if index == lineCharsCount:
            continue

        cutLine = line[index:].strip()
        transferredLines.append(f"# {cutLine}")

    return "\n".join(transferredLines) + "\n"


def _IsFileValid(
    file: str,
    extensions: list[str],
) -> bool:
    """
    Check if the file has a valid extension.

    Arguments
    ---------
    file : str
        The file path.

    extensions : list[str]
        The list of valid extensions.

    Returns
    -------
    bool
        True if the file has a valid extension, False otherwise.
    """

    _, ext = os.path.splitext(file)
    return ext in extensions


def _LoadFilesRecursively(
    finalFiles: set[str],
    folder: str,
    extensions: list[str],
) -> set[str]:
    """
    Load all files in the folder recursively.

    Arguments
    ---------
    finalFiles : list[str]
        The list of files to append to.

    folder : str
        The folder path.

    extensions : list[str]
        The list of valid extensions.

    Returns
    -------
    list[str]
        The list of files.
    """

    if not os.path.exists(folder):
        logger.warning(f'Folder path "{folder}" does not exist, skipping...')
        return finalFiles

    for root, _folders, files in os.walk(folder):
        for file in files:
            fullPath = os.path.join(root, file)

            if _IsFileValid(fullPath, extensions):
                relPath = os.path.relpath(fullPath, SYSTEM.BASE_DIR)
                finalFiles.add(relPath)

        for _folder in _folders:
            _LoadFilesRecursively(finalFiles, os.path.join(root, _folder), extensions)

    return finalFiles


def _AllDependenciesFiles(
    dependencies: list[str],
    extensions: list[str],
) -> list[str]:
    """
    List all files in the dependencies (folders) recursively.

    Arguments
    ---------
    dependencies : list[str]
        The list of dependency folders.

    extensions : list[str]
        The list of extensions to consider.

    Returns
    -------
    list[str]
        The list of all files in the dependencies.
    """

    allFiles: set[str] = set()
    for dep in dependencies:
        allFiles.update(
            _LoadFilesRecursively(
                allFiles,
                os.path.join(SYSTEM.BASE_DIR, dep),
                extensions,
            )
        )

    return list(allFiles)


def GenerateBindings(
    binding: Binding,
    testContent: str | None = None,
) -> tuple[str, list[str]]:
    """
    The tools creating the binding files.

    Arguments
    ---------
    binding : Binding
        The binding configuration.

    testContent: str(optional)
        Just be used for testing

    force : bool(optional)
        Force regeneration of the binding file.
    """
    filePath = os.path.join(SYSTEM.BASE_DIR, binding.file)
    templatePath = os.path.join(SYSTEM.BASE_DIR, binding.template)
    outputPath = os.path.join(SYSTEM.BASE_DIR, binding.output)

    dependenciesFiles: list[str] = []

    if binding.dependencies is not None:
        assert (
            binding.extensions is not None
        ), f'Binding "{binding.file}" has dependencies but no extensions specified.'

    if testContent is None and binding.dependencies is not None:
        assert binding.extensions is not None

        dependenciesFiles = _AllDependenciesFiles(
            binding.dependencies,
            binding.extensions,
        )

        logger.debug(
            f'All dependency files for binding "{binding.file}": \n{str(dependenciesFiles).replace(", ", ",\n\t").replace("[", "[\n\t").replace("]", "\n]")}'
        )

        hasModified = False

        for depFile in dependenciesFiles:
            if IsFileModified(depFile):
                hasModified = True
                break

        if (
            not hasModified
            and not IsFileModified(binding.template)
            and os.path.exists(outputPath)
        ):
            logger.debug(
                f'Binding file "{binding.file}" and its dependencies have not been modified, skipping...'
            )
            return ("", [])

    assert os.path.exists(filePath), f'Binding file "{filePath}" does not exist.'
    assert os.path.exists(
        templatePath
    ), f'Template path "{templatePath}" does not exist.'

    logger.debug(f'Analysing binding file "{binding.file}"...')
    global parser
    parser = Parser(filePath=filePath, content=testContent)
    parser.Parse()

    logger.debug(f'Generating binding file "{binding.output}"...')
    with open(templatePath, "r") as f:
        templateContent = f.read()

    env = Environment(loader=FileSystemLoader(SYSTEM.BASE_DIR))
    template = env.from_string(templateContent)

    content = template.render(
        structs=parser.Structs,
        enums=parser.Enums,
        typedefs=parser.Typedefs,
        functions=parser.Functions,
        cTypeConvert=_CTypeConvert,
        getFunctionParameters=_GetFunctionParameters,
        convertRawCCommentToPythonDocstring=_ConvertRawCCommentToPythonDocstring,
        convertRawCCommentToPythonComment=_ConvertRawCCommentToPythonComment,
    )

    if testContent is None:
        outputFolder = os.path.dirname(outputPath)
        Path(outputFolder).mkdir(parents=True, exist_ok=True)

        with open(outputPath, "w") as f:
            f.write(content)

        dependenciesFiles.append(binding.template)

    logger.info(
        f'Generated binding file "{binding.output}" from "{binding.file}" using template "{binding.template}".'
    )

    return (content, dependenciesFiles)
