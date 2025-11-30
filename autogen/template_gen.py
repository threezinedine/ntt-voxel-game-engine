import os
from models import Template
from jinja2 import Template as JinjaTemplate
from utils import (
    SYSTEM,
    logger,
    TEMPLATE_DATA,
    IsFileModified,
    UpdateFileStamp,
    AllDependenciesFiles,
)


def GenerateTemplate(template: Template) -> tuple[str, list[str]]:
    """
    The tools creating the template files.

    Arguments
    ---------
    template : Template
        The template configuration.
    """
    templatePath = os.path.join(SYSTEM.BaseDir, template.file)

    allDependencies = []

    if template.dependencies is not None:
        assert (
            template.extensions is not None
        ), f'Template "{template.file}" has dependencies but no extensions specified.'

        allDependencies = AllDependenciesFiles(
            template.dependencies,
            template.extensions,
        )

        logger.debug(
            f'All dependency files for template "{template.file}": \n{str(allDependencies).replace(", ", ",\n\t").replace("[", "[\n\t").replace("]", "\n]")}'
        )

    if not os.path.exists(templatePath):
        logger.warning(f'Template file "{template.file}" does not exist, skipping...')
        return "", []

    outputFiles = [template.file[:-3]]  # remove .in extension

    if template.outputs is not None:
        outputFiles = template.outputs

    fullOutputPaths = [
        os.path.join(SYSTEM.BaseDir, outputFile) for outputFile in outputFiles
    ]

    isAnyDependencyModified = False
    for depFile in allDependencies:
        if IsFileModified(depFile):
            isAnyDependencyModified = True
            break

    if (
        not isAnyDependencyModified
        and not IsFileModified(template.file)
        and all(os.path.exists(fullOutputPath) for fullOutputPath in fullOutputPaths)
    ):
        logger.debug(
            f'Template file "{template.file}" has not been modified, skipping...'
        )
        return "", []

    with open(templatePath, "r") as f:
        templateContent = f.read()
        jinjaTemplate = JinjaTemplate(templateContent)
        renderedContent = jinjaTemplate.render(**TEMPLATE_DATA)

    for outputFile, fullOutputPath in zip(outputFiles, fullOutputPaths):
        os.makedirs(os.path.dirname(fullOutputPath), exist_ok=True)

        with open(fullOutputPath, "w") as f:
            f.write(renderedContent)

        logger.info(f'Generated file "{outputFile}" from template "{template.file}".')

    UpdateFileStamp(template.file)

    return renderedContent, allDependencies
