#
#    Copyright (C) 2013 Stanislav Bohm
#
#    This file is part of Kaira.
#
#    Kaira is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, version 3 of the License, or
#    (at your option) any later version.
#
#    Kaira is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Kaira.  If not, see <http://www.gnu.org/licenses/>.
#


import gtkutils
import gtk

def tracefn_dialog(mainwindow, fn_name, return_type):
    builder = gtkutils.load_ui("tracefn-dialog")
    dlg = builder.get_object("tracefn-dialog")

    try:
        name = builder.get_object("name")
        name.set_text(fn_name)

        return_int = builder.get_object("return_int")
        return_double = builder.get_object("return_double")
        return_string = builder.get_object("return_string")

        if return_type == "int":
            return_int.set_active(True)
        elif return_type == "double":
            return_double.set_active(True)
        else:
            return_string.set_active(True)

        dlg.set_title("Trace function")
        dlg.set_transient_for(mainwindow)
        if dlg.run() == gtk.RESPONSE_OK:
            if return_int.get_active():
                return_type = "int"
            elif return_int.get_active():
                return_type = "double"
            else:
                return_type = "std::string"
            return (name.get_text(), return_type)
        return None
    finally:
        dlg.destroy()