# hhhconf

Config tool for tint2. Written for @hhhorb

## Build and install

```
make install
```

## Scrot

[![2019-06-29-214211_1024x600_scrot.md.png](https://cdn.scrot.moe/images/2019/06/29/2019-06-29-214211_1024x600_scrot.md.png)](https://scrot.moe/image/xY1U0)

```sh
hhhconf i <<< 0
hhhconf f
hhhconf 3o <<< 0
hhhconf 3p <<< 2
hhhconf 3nt <<< 5
```

## Usage

hhhconf is an interactive too, so just follow instructions.

```
hhhconf
```

Alternatively, use `hhhconf-t2` directly. Run with `-h` for full help message.
Here follow some examples:

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

