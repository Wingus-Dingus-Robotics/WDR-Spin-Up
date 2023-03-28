# WDR Spin Up: Competition code

You spin my head right 'round (right 'round)

Like a record baby (right right 'round 'round)

## Project setup

### 1) V5 SDK and build tools

I copied VEXcode sdk and tool folders into the vex folder of this project. Currently gitignored.

### 2) IDE

List of VSCode extensions:

- VEX Robotics
- clangd
- Doxygen Documentation Generator
  - Not required
  - Helps generate documentation comments (Type `/**<enter>`)
- C-mantic
  - Not required
  - Has handy commands like "Generate header guards"

Generate compilation database to make clangd intellisense work.

```sh
chmod +x scripts/cross-compile.sh
./scripts/cross-compile.sh bear
```

Script only works on machines with bear installed (only available for linux/mac currently).
