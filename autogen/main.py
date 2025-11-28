import json
import logging
import argparse
from models import Settings
from dacite import from_dict
from binding import GenerateBindings
from utils import logger, ClearCache
from template_gen import GenerateTemplate


def main():
    parser = argparse.ArgumentParser(description="Autogen script")
    parser.add_argument(
        "--reload",
        "-r",
        action="store_true",
        help="Force to re-generate files even if they are not modified",
    )
    parser.add_argument(
        "--verbose",
        "-v",
        action="store_true",
        help="Enable verbose logging",
    )

    args = parser.parse_args()

    if args.verbose:
        logger.setLevel(logging.DEBUG)
    else:
        logger.setLevel(logging.INFO)

    if args.reload:
        logger.info("Force flag detected, re-generating all files...")
        ClearCache()

    settings: Settings | None = None

    with open("settings.json") as f:
        settings = from_dict(data_class=Settings, data=json.loads(f.read()))

    for template in settings.templates:
        GenerateTemplate(template)

    for binding in settings.bindings:
        GenerateBindings(binding, force=args.reload)


if __name__ == "__main__":
    main()
