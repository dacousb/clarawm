# clarawm
> *a simple tiling/floating window manager*

clarawm is designed to be a simple tiling/floating window manager, fast and with a light and simple code base, so that anyone can understand the code.

![screenshot](image.png)

## Shortcuts (keys)

The default configured modifier is Mod4 (⌘). This can be combined with the following keys to achieve different results:

|   | Key    | Action        |
|---|--------|---------------|
| ⌘ | Return | Open xterm    |
| ⌘ | d      | Open dmenu    |
| ⌘ | q      | Close window  |
| ⌘ | k      | Kill wm       |
| ⌘ | m      | Change mode   |
| ⌘ | MouseL | Move window   |
| ⌘ | MouseR | Resize window |

The available modes (toggleable with Mod4+m) are:
- `[]=` Tiling
- `<><` Floating

## To do
Looking to get involved in the project or just know what do clarawm need to improve? Here it is:
- Fix bugs in tiling mode, which detects hidden windows.
- Better tiling (vertical and horizontal).
- Interactive tiling.
- More information on the top menu.
- Try to introduce virtual workspaces.

Note: clarawm is not intended to replace other window managers, I was just looking for a clean and simple manager for me.