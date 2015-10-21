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
from runinstance import RunInstance, TransitionFire

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


        TIME_STAMP = { "name" : "timestamp", "type" : "int" }
        PROCESS_ID = { "name" : "pid", "type" : "int" }
        ACTIVITY = { "name" : "activity", "type" : "string" }

        NAME = {"name" : "name", "type" : "string" }
        TRANSITION_ID = {"name" : "trans-id", "type" : "int" }

        @staticmethod
        def createAttr(definition, value=""):
            return xes.Attribute(type=definition["type"],
                                 key=definition["name"], value=str(value))

        def __init__(self, tracelog, xes_trace):
            RunInstance.__init__(self,
                                 tracelog.project,
                                 tracelog.process_count)

            self.xes_trace = xes_trace

        def get_log(self):
            return self.xes_trace

        def transition_fired(self, process_id, time, transition_id, values):
            transition = self.net.item_by_id(transition_id)

            e = xes.Event()
            e.add_attribute(self.createAttr(self.TIME_STAMP, time))
            e.add_attribute(self.createAttr(self.PROCESS_ID, process_id))
            e.add_attribute(self.createAttr(self.ACTIVITY,
                                             "fire-{0}".format(transition_id)))

            # optional attributes
            e.add_attribute(self.createAttr(self.TRANSITION_ID, transition_id))
            e.add_attribute(self.createAttr(self.NAME, transition.get_name_or_id()))
            self.xes_trace.add_event(e)

            RunInstance.transition_fired(self, process_id,
                                         time, transition_id, values)

        def event_send(self, process_id, time, target_id, size, edge_id):
            assert isinstance(self.last_event_activity, TransitionFire)
            transition_fire = self.last_event_activity

            e = xes.Event()
            e.add_attribute(self.createAttr(self.TIME_STAMP, time))
            e.add_attribute(self.createAttr(self.PROCESS_ID, process_id))
            e.add_attribute(self.createAttr(
                self.ACTIVITY, "send/{0}/{1}".format(process_id, target_id)))

            # optional attributes
            e.add_attribute(self.createAttr(self.TRANSITION_ID,
                                             transition_fire.transition.id))
            e.add_attribute(self.createAttr(
                { "name" : "tpid", "type" : "int" }, value=str(target_id)))
            e.add_attribute(self.createAttr(
                { "name" : "msg-size", "type": "int" }, size))
            self.xes_trace.add_event(e)

            RunInstance.event_send(self, process_id,
                                   time, target_id, size, edge_id)

        def event_receive(self, process_id, time, origin_id):
            e = xes.Event()
            e.add_attribute(self.createAttr(self.TIME_STAMP, time))
            e.add_attribute(self.createAttr(self.PROCESS_ID, process_id))
            e.add_attribute(self.createAttr(
                self.ACTIVITY, "receive/{0}/{1}".format(process_id, origin_id)))

            # optional attributes
            e.add_attribute(self.createAttr(
                { "name" : "opid", "type" : "int" }, value=str(origin_id)))

            self.xes_trace.add_event(e)

            RunInstance.event_receive(self, process_id, time, origin_id)


    name = "Tracelogs to XES"
    description = "Converts a kaira tracelog into a eXtensible Event Stream"\
                  " (XES) format"

    parameters =[ extensions.Parameter("Tracelog", datatypes.t_tracelog, True) ]

    def run(self, app, tracelogs):
        log = xes.Log("2.0")

        # set default values
        log.add_global_event_attribute(
            self.XESRunInstance.createAttr(self.XESRunInstance.TIME_STAMP, -1))
        log.add_global_event_attribute(
            self.XESRunInstance.createAttr(self.XESRunInstance.PROCESS_ID, -1))
        log.add_global_event_attribute(
            self.XESRunInstance.createAttr(self.XESRunInstance.ACTIVITY, ""))

        for tracelog in tracelogs:
            trace = xes.Trace()
            xesri = self.XESRunInstance(tracelog, trace)
            tracelog.execute_all_events(xesri)
            log.add_trace(trace)
            log.add_classifier(xes.Classifier("activity_uid",
                                              "pid activity"))

        return extensions.Source("Tracelog for process mining",
                                 datatypes.t_xes,
                                 log)

extensions.add_operation(TracelogToXES)
