# what

It's a test of using uinput to create a fake input device (in this case, a keyboard),
and send events using it. It's mostly ripped from the uinput documentation, just
putting it here so I can refer back to it and hack it as needed.

To use it, make sure your kernel has CONFIG_INPUT_UINPUT enabled (and loaded, if it's a module).

For more information, see https://www.kernel.org/doc/html/latest/input/uinput.html
