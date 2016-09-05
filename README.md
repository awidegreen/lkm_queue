# lkm_queue - A simple LKM queueing device

This is a simple implementation of an LKM (Linux Kernel Module) queueing device
using a misc char-device.


## Requirements

Make sure that you have the kernel-dev dependencies and a gcc installed. 

## Usage

Build everything:

```sh
$ make
```

Build and load the kernel module:

```sh
$ make reload_mod
```

Run the `writer`:

```sh
# continously writes some 'signals' to the queue.
$ sudo ./writer
```

```sh
# continously read 'signals' from the queue.
$ sudo ./reader
```

## Notes

The `/dev/lkm_queue` can only be open as root, hence `writer` and `reader` need
to be started with `sudo`. Otherwise, define some udev rules to make the device
accessible without root privileges.

## Credits 
The LKM implementation is heavily based on the [Be a kernel hacker](https://www.linuxvoice.com/be-a-kernel-hacker/)
tutorial. 

## License

Copyright (C) 2016 by Armin Widegreen

This is free software, licensed under The [MIT License](LICENSE).
