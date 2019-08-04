# hhhconf

Config tool for tint2. Written for @hhhorb

## Build and install

```
make install
```

## Usage

hhhconf is an interactive too, so should be self explanatory.

```
hhhconf
```

Alternatively, use `hhhconf-t2` directly. Run with `-h` for full help message.

## Scrot

[![2019-06-29-214211_1024x600_scrot.md.png](https://cdn.scrot.moe/images/2019/06/29/2019-06-29-214211_1024x600_scrot.md.png)](https://scrot.moe/image/xY1U0)

Reproduce by taking the step below.

Although these steps have been written to be run from a bash command line, each
instruction (e.g. i, f, 3) can be run at the interactive hhhconf prompt.

The `<<< x` syntax means that we supply `x` to a follow-up question.

```sh
# import a tint2rc config file (one with task background and border colours)
hhhconf i <<< 0

# set whatever font gtk3 uses (e.g. what you've set with lxappearance)
hhhconf f

# Use palette number 3 (which is just a hard coded palette).
# Use 4 instead if you want to base the palette your nitrogen wallpaper
hhhconf 3

# Set panel and task colours
hhhconf o <<< 0
hhhconf p <<< 2
hhhconf n <<< 5

# Restart tint2
hhhconf t
```

## Examples

Get value of `task_font` (e.g. 'Sans 10')

```sh
hhhconf-t2 task_font
```

Set `panel_items` to TCS

```sh
hhhconf-t2 panel_items TCS
```

Set task font name but keep size (e.g. 'Futura Bk BT 10')

```sh
hhhconf-t2 -p task_font "Futura Bk BT"
```

Set `background_color` associated with `task_active`

```sh
hhhconf-t2 -s task_active background_color "#ff0000 100"
```

Add missing font variables excl those associated with 'execp' and 'button' plugins

```sh
hhhconf-t2 -F
```

