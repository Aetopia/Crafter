# Crafter
A zero dependency lightweight injector for Minecraft: Bedrock Edition.

## Features

- Reliably inject dynamic link libraries into Minecraft: Bedrock Edition.

- Adds relevant permissions to make dynamic link libraries readable & executable within Minecraft: Bedrock Edition.

## Building
1. Install [MSYS2](https://www.msys2.org/) & [UPX](https://upx.github.io/) for optional compression.
2. Update the MSYS2 Environment until there are no pending updates using:

    ```bash
    pacman -Syu --noconfirm
    ```

3. Install GCC & [MinHook](https://github.com/TsudaKageyu/minhook) using:

    ```bash
    pacman -S mingw-w64-ucrt-x86_64-gcc --noconfirm
    ```

3. Make sure `<MSYS2 Installation Directory>\ucrt64\bin` is added to the Windows `PATH` environment variable.
4. Run [`Build.cmd`](src/Build.cmd).