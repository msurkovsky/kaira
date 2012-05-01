#
#    Copyright (C) 2011 Ondrej Meca
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

from project import Project, NativeExternType, Function

class ProjectJava(Project):

    def __init__(self, file_name):
        Project.__init__(self, file_name)
        self.build_options = {
            "JFLAGS" : "-g",
            "JC" : "javac",
            "LIBS" : ""
        }

    @classmethod
    def get_extenv_name(self):
        return "Java"

    def is_library(self):
        return False

    def get_native_extern_type_class(self):
        return ExternTypeJava

    def get_function_class(self):
        return FunctionJava

    def get_syntax_highlight_key(self):
        """return language for GtkSourceView"""
        return "java"

    def get_head_comment(self):
        return "/* The code from 'head' is included at the beginning of generated project.\n" \
               " * The main purpose is to put here definitions (#includes, classes, ...)\n" \
               " * that can be used everywhere. (functions in transitions and places).\n" \
               " */\n\n"

    def get_source_file_patterns(self):
        return ["*.java"]

class ExternTypeJava(NativeExternType):

    def __init__(self, name = "", raw_type = "", transport_mode = "Disabled"):
        NativeExternType.__init__(self, name, raw_type, transport_mode)


    def get_default_function_code(self):
        return "\treturn \"" + self.name + "\";\n"

    def get_function_declaration(self, name):
        if name == "getstring":
            return "String getstring(" + self.raw_type + " obj)"
        elif name == "getsize":
            return "int getsize(" + self.raw_type + " obj)"
        elif name == "pack":
            return "void pack(CaPacker packer, " + self.raw_type + " obj)"
        elif name == "unpack":
            return self.raw_type + " unpack(CaUnpacker unpacker)"


class FunctionJava(Function):

    def __init__(self, id = None):
        Function.__init__(self, id)

    def get_function_declaration(self):
        return self.get_raw_return_type() + " " + self.name + "(" + self.get_raw_parameters() + ")"

    def get_raw_parameters(self):
        p = self.split_parameters()
        if p is None:
            return "Invalid format of parameters"
        else:
            params_str =    [ self.project.type_to_raw_type(t) + " " + n for (t, n) in p ]
            if self.with_context:
                params_str.insert(0, "CaContext ctx")
            return ", ".join(params_str)

    def get_raw_return_type(self):
        return self.project.type_to_raw_type(self.return_type)

