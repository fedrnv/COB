python
import gdb


class InfoPrettyPrinterCompat(gdb.Command):
    """Compatibility alias for IDEs that call 'info pretty-printer'."""

    def __init__(self):
        super(InfoPrettyPrinterCompat, self).__init__(
            "info pretty-printer",
            gdb.COMMAND_STATUS,
        )

    def invoke(self, arg, from_tty):
        suffix = (" " + arg) if arg else ""
        gdb.execute("info pretty-printers" + suffix, from_tty)


InfoPrettyPrinterCompat()
end
