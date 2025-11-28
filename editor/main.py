from Engine import (
    meedPlatformPrint,
    meedPlatformSetConsoleConfig,
    MEEDConsoleColor,
    MEEDPlatformConsoleConfig,
)

config = MEEDPlatformConsoleConfig()
config.color = MEEDConsoleColor.MEED_CONSOLE_COLOR_GREEN

meedPlatformSetConsoleConfig(config)
meedPlatformPrint("Hello, Meed Platform!")
