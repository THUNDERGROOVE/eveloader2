# eveloader2
eveloader2 is a launcher and dll injection tool for EVE online build 360229.


## running eveloader2
### interactive setup
by default if you just run `eveloader2.exe` for the first time it will prompt you if you want to run the interactive setup.
1. it will ask for your game installation, navigate to it and click Ok.
2. next it will ask if you want to enable the filesystem overlay, I totally recommend it, but you may want to see the [fsmapper section](#fsmapper)
3. the game should open, at this point I'd recommend looking at making [shortcuts](#shortcuts)
```shell
eveloader2.exe -u <username> -p <password> -h <host>
```
please note: eveloader2 does not support non-standard ports at this time.  I can add this if there's a need out there. 
## configuration
sample configuration is below and is shared between both eveloader2, and it's injection dll.
```ini
[eveloader]
# full path to your installation
eve_installation = D:\Crucible
# this enables the python console - this is useful for developers
use_console = true
# this enables fsmapper - this allows modification of the client without tampering with your client
use_fsmapper = true
```
## patching
every time eveloader2 runs it scans the game's blue.dll file and attempt to help you patch it if needed.  it can tell between the following states.

- stock from CCP
- [stschake's blue_patcher](https://github.com/stschake/blue_patcher)
- eveloader2's custom patch.

1. if blue.dll is stock, it will ask if you want to patch it.
2. if blue.dll is blue_patcher or unknown, it will tell you to restore a backup before proceeding
3. if blue.dll is our patch, we proceed
4. eveloader2 will attempt patch `common.ini` to set `crytpoPack=Placebo` 

## fsmapper
fsmapper allows eveloader2 and **you** to modify the client without directly tampering with the installation.  this is done by redirecting the game's calls to `_CreateFileW` which effectively allows us to redirect that to anywhere else on the disk.

when enabled a few things happen

1. eveloader2 will copy blue.dll from the game installation to `C:\ProgramData\eveloader`
2. eveloader2 will redirect any modifications to game files to its program data folder and tell the client to read from there as well.

below are some files we support.
- `common.ini` - this is needed to modify the cryptoPack config
- `start.ini` - this is needed to modify the default host and port
- `bin/blue.dll` - this is the special patched `blue.dll` file
- `script/compiled.code` - this allows the user to modify the game's python code
- `script/devtools.code` - this allows the user to inject their own devtools package
- `lib/carbonlib.ccp` - this allows the user to inject their own code here
- `lib/carbonstdlibb.ccp` - this allows the user to inject their own code here
- `lib/evelib.ccp` - this allows the user to inject their own code here

you will also get logs stored here `eveloader2.log` and `eveloader2_dll.log`.  If you run into any issues these will be helpful.
## custom/dynamic patches
you can tell eveloader2 to do additional patching via the `patches.ini` file.  an example of that is below.

```ini
[patches]
patch_list=public_key

[public_key]
name="Patch public key"
# original_data and patched_data should be relative paths to files.  
# the files should always be the same size or an error will be shown.
# we will find the original_data contents in blue.dll and overwrite it with patched_data contents
original_data=./patches/original_pub_key
patched_data=./patches/evecc.keys.pub
```
## shortcuts
if you create a shortcut to `eveloader2.exe` if you right-click, properties you can modify the Target field similar to below to automatically log in to your server.
```shell
"C:\Users\nick\source\repos\eveloader2\cmake-build-release\eveloader2.exe" -u <username> -p <password> -h <host>
```
## build requirements
i get it to build in clion 2022, you can probably use command line cmake if you desired
you probably need
- .net framework 4.7.2 developer pack for easyhook

## disclaimer
eveloader2 is an educational project to learn more about the internals of the EVE online client.  I do not condone any of eveloader2 to be used with any modern clients.