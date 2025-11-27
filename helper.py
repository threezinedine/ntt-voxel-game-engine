from config import (
    Args,
    logger,
    Build,
    SYSTEM,
    RunEditor,
    RunAutogen,
    RunExample,
    SetupEditor,
    SetupLogging,
    CreateEnvironment,
    InstallPythonDependencies,
    ValidateCommandExist,
    InstallCDependencies,
    InstallEditorDependencies,
)


def main():
    args = Args()

    SetupLogging(**args.Args)

    logger.debug("Start checking the system health...")
    ValidateCommandExist(SYSTEM.PythonCommand)
    ValidateCommandExist("cmake")
    ValidateCommandExist("clang")
    ValidateCommandExist("git")
    logger.info("System health check passed.")

    CreateEnvironment(dir="autogen", **args.Args)
    InstallCDependencies()
    SetupEditor()
    RunAutogen(**args.Args)

    if args.IsBuild:
        if args.IsCProject:
            Build(**args.Args)
    elif args.IsPackage:
        if args.IsPythonProject:
            InstallPythonDependencies(**args.Args)
        elif args.IsCProject:
            InstallEditorDependencies(**args.Args)
        elif args.IsJSProject:
            InstallEditorDependencies(**args.Args)
        else:
            raise NotImplementedError(
                f'Packaging for project "{args.args.project}" is not implemented.'
            )
    elif args.IsRun:
        Build("engine", **args.Args)
        RunEditor(**args.Args)
    elif args.IsRunExample:
        RunExample(**args.Args)
    elif args.IsRunAutogen:
        pass
    else:
        raise NotImplementedError(
            f"The command {args.args.command} is missing or not implemented."
        )


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        logger.error(f"An error occurred: {e}")
        exit(1)
