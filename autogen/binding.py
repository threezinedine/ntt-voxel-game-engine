import os
from pathlib import Path
from utils import SYSTEM
from analyze import Parser
from models import Binding
from jinja2 import Environment
from jinja2 import FileSystemLoader
from utils import IsFileModified, UpdateFileStamp, logger
from utils.cache import ClearCache


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
    else:
        return cType


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
    force: bool = False,
) -> str:
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

    if force:
        ClearCache()

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
            return ""

    assert os.path.exists(filePath), f'Binding file "{filePath}" does not exist.'
    assert os.path.exists(
        templatePath
    ), f'Template path "{templatePath}" does not exist.'

    logger.debug(f'Analysing binding file "{binding.file}"...')
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
    )

    if testContent is None:
        outputFolder = os.path.dirname(outputPath)
        Path(outputFolder).mkdir(parents=True, exist_ok=True)

        with open(outputPath, "w") as f:
            f.write(content)

        if binding.dependencies is not None:
            for depFile in dependenciesFiles:
                UpdateFileStamp(depFile)

        UpdateFileStamp(binding.template)

    logger.info(
        f'Generated binding file "{binding.output}" from "{binding.file}" using template "{binding.template}".'
    )

    return content
