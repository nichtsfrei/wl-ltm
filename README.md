# wl-lid-toggle-monitor

Daemon to toggle a defined display when lid is closed for wayland compositors.

## Building

Install dependencies:

* meson (compile-time dependency)
* wayland

Then run:
```
meson build -Ddisplay=eDP-1
ninja -C build
build/wl-ltm
```

The wayland protocol is stolen from [wlr-randr](https://sr.ht/~emersion/wlr-randr/).


### Options

| Name | Default | Description |
| -- | -- | -- |
| pidlocation | /tmp/wl-ltm.pid | location to store the pid file |
| lidstatelocation | /proc/acpi/button/lid/LID/state | location of the lid state file |
| display | eDP-1 | display name to toggle |


## License

MIT
