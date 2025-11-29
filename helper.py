from config import (
    Args,
    logger,
    SYSTEM,
    RunEditor,
    RunAutogen,
    RunExample,
    BuildEditor,
    SetupLogging,
    OpenDesigner,
    BuildCProject,
    RunTestEngine,
    RunPythonTest,
    RunApplication,
    CreateEnvironment,
    BuildEngineWebLib,
    RunEditorConvertUI,
    ValidateCommandExist,
    InstallCDependencies,
    InstallPythonDependencies,
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
    CreateEnvironment(dir="editor", **args.Args)
    InstallCDependencies()
    RunAutogen(**args.Args)
    RunEditorConvertUI(**args.Args)

    if args.IsBuild:
        if args.IsCProject:
            BuildCProject(**args.Args)
        elif args.IsPythonProject:
            BuildEditor(**args.Args)
    elif args.IsPackage:
        if args.IsPythonProject:
            InstallPythonDependencies(**args.Args)
        elif args.IsCProject:
            # InstallCDependencies(**args.Args)
            pass
        else:
            raise NotImplementedError(
                f'Packaging for project "{args.args.project}" is not implemented.'
            )
    elif args.IsRunTest:
        if args.IsPythonProject:
            RunPythonTest(**args.Args)
        elif args.IsCProject:
            BuildCProject(**args.Args)
            RunTestEngine(**args.Args)
        else:
            raise NotImplementedError(
                f'Testing for project "{args.args.project}" is not implemented.'
            )
    elif args.IsRunDesigner:
        OpenDesigner(**args.Args)
    elif args.IsRun:
        if args.IsPythonProject:
            BuildCProject(**(args.Args | dict(project="engine")))
            RunEditor(**args.Args)
        else:
            RunApplication(**args.Args)
    elif args.IsRunExample:
        RunExample(**args.Args)
    elif args.IsRunAutogen:
        pass
    elif args.IsRunWebRuntime:
        BuildEngineWebLib(**args.Args)
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
