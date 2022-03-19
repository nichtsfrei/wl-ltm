# wl-lid-toggle-monitor

Utility to toggle a defined display when lid is closed for wayland compositors.


## Building

Install dependencies:

* meson (compile-time dependency)
* wayland

Then run:
```
meson build
ninja -C build
build/wlr-randr
```

Shamelessly stolen from [wlr-randr](https://sr.ht/~emersion/wlr-randr/).

## License

MIT
