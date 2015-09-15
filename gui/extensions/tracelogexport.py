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
from runinstance import RunInstance

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

    class XESRunInstance(RunInstance):

        def __init__(self, tracelog, xes_trace):
            RunInstance.__init__(self,
                                 tracelog.project,
                                 tracelog.process_count)

            self.xes_trace = xes_trace

        def get_log(self):
            return self.xes_trace

        def transition_finished(self, process_id, time):
            activity = self.activites[process_id]
            e = xes.Event()
            e.add_attribute(xes.Attribute(
                type="int",
                key="timestamp",
                value=str(activity.time)
            ))
            e.add_attribute(xes.Attribute(
                type="id",
                key="id",
                value=str(activity.transition.id)
            ))
            e.add_attribute(xes.Attribute(
                type="int",
                key="pid",
                value=str(process_id)
            ))
            # optional attributes
            e.add_attribute(xes.Attribute(
                type="string",
                key="name",
                value=activity.transition.get_name_or_id()
            ))
            e.add_attribute(xes.Attribute(
                type="int",
                key="duration",
                value=(str(time - activity.time))
            ))
            self.xes_trace.add_event(e)

            RunInstance.transition_finished(self, process_id, time)

    name = "Tracelogs to XES"
    description = "Converts a kaira tracelog into a eXtensible Event Stream"\
                  " (XES) format"

    parameters =[ extensions.Parameter("Tracelog", datatypes.t_tracelog, True) ]

    def run(self, app, tracelogs):
        log = xes.Log()
        for tracelog in tracelogs:
            trace = xes.Trace()
            xesri = self.XESRunInstance(tracelog, trace)
            tracelog.execute_all_events(xesri)
            log.add_trace(trace)
            log.add_classifier(xes.Classifier("activity_uid", "id pid"))
            log.add_classifier(xes.Classifier("activity_name", "name pid"))

        return extensions.Source("Tracelog for process mining",
                                 datatypes.t_xes,
                                 log)

extensions.add_operation(TracelogToXES)
