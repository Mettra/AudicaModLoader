# Audica PC Modloader

## Introduction

A modloader for PC DLLs for Audica.

### How it works

This modloader works as a standalone DLL (`IPHLPAPI.dll`). It get loaded by Audica when it starts up, and then loads mods from the `Mods` directory (which it will create if it does not exist) and applies them to Audica.

### How to use it

* Download the latest files from [the releases page](https://github.com/Mettra/AudicaModLoader/releases)
* Extract everything as `Audica.exe` and `GameAssembly.dll` files
* Running the game once should create a `Mods` folder for you, which you can then place your mods in.

## Building

```bash
git clone https://github.com/Mettra/AudicaModLoader.git
cd AudicaModLoader
mkdir build
cd build
cmake ../ -DCMAKE_GENERATOR_PLATFORM=x64
```
`AudicaModLoader.sln` should be generated under the `AudicaModLoader/build` folder.

1. Building the `AudicaModDoorstop` `Visual Studio` project will output a `IPHLPAPI.dll`, which should be placed next to `Audica.exe` and `GameAssembly.dll`

## Developing

After running the above steps, there will also be a `SampleMod` project, which is setup to output a DLL that you can place in the `Mods` folder. It's a silly mod that lets you throw your guns at the targets to shoot them, but it's complex enough to show off a lot of the functionality you will need for developting mods.