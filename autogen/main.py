import os
import json
import argparse
from jinja2 import Template as JinjaTemplate
from models import Settings
from dacite import from_dict
from utils import (
    SYSTEM,
    TEMPLATE_DATA,
    logger,
    IsFileModified,
    UpdateFileStamp,
    ClearCache,
)


def main():
    parser = argparse.ArgumentParser(description="Autogen script")
    parser.add_argument(
        "--reload",
        "-r",
        action="store_true",
        help="Force to re-generate files even if they are not modified",
    )

    args = parser.parse_args()

    if args.reload:
        logger.info("Force flag detected, re-generating all files...")
        ClearCache()

    settings: Settings | None = None

    with open("settings.json") as f:
        settings = from_dict(data_class=Settings, data=json.loads(f.read()))

    for template in settings.templates:
        if not IsFileModified(template.filePath):
            logger.debug(
                f'Template file "{template.filePath}" has not been modified, skipping...'
            )
            continue

        templatePath = os.path.join(SYSTEM.BASE_DIR, template.filePath)

        if not os.path.exists(templatePath):
            logger.warning(
                f'Template file "{template.filePath}" does not exist, skipping...'
            )
            continue

        outputFile = template.filePath[:-3]  # remove .in extension

        if template.output is not None:
            outputFile = template.output

        fullOutputPath = os.path.join(SYSTEM.BASE_DIR, outputFile)

        with open(templatePath, "r") as f:
            templateContent = f.read()
            jinjaTemplate = JinjaTemplate(templateContent)
            renderedContent = jinjaTemplate.render(**TEMPLATE_DATA)

        with open(fullOutputPath, "w") as f:
            f.write(renderedContent)

        logger.info(
            f'Generated file "{outputFile}" from template "{template.filePath}".'
        )

        UpdateFileStamp(template.filePath)


if __name__ == "__main__":
    main()
