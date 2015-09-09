#
#    Copyright (C) 2013 Stanislav Bohm
#                  2014 Martin Surkovsky
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


import extensions
import datatypes
import utils
import exportri
import xes
from exportri import ExportRunInstance

class TracelogExport(extensions.Operation):

    name = "Tracelog export"
    description = "Export data from tracelog into a table"

    parameters = [ extensions.Parameter("Tracelog", datatypes.t_tracelog) ]

    def run(self, app, tracelog):

        settings = exportri.run_assistant(app, tracelog);
        if settings is None:
            return

        ri = ExportRunInstance(tracelog, *settings)
        tracelog.execute_all_events(ri)
        return extensions.Source("Tracelog Table",
                                 datatypes.t_table,
                                 ri.get_table())

extensions.add_operation(TracelogExport)

class TracelogToXES(extensions.Operation):

    name = "Kaira Tracelog to XES"
    description = "Converts a kaira tracelog into a eXtensible Event Stream"\
                  " (XES) format"

    # TODO: work with more than one paramter
    #parameters = [ extensions.Parameter("Tracelog", datatypes.t_tracelog) ]
    parameters =[]

    #def run(self, app, tracelog):
    def run(self, app):
        log = xes.Log()
        print log

extensions.add_operation(TracelogToXES)
