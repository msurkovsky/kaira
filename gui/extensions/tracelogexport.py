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
            e.add_attribute(xes.Attribute(
                type="int",
                key="timestamp",
                value=str(time)
            ))
            e.add_attribute(xes.Attribute(
                type="int",
                key="trans-id",
                value=str(transition_id)
            ))
            e.add_attribute(xes.Attribute(
                type="int",
                key="pid",
                value=str(process_id)
            ))
            e.add_attribute(xes.Attribute(
                type="string",
                key="activity",
                value="fire"
            ))

            # optional attributes
            e.add_attribute(xes.Attribute(
                type="string",
                key="name",
                value=transition.get_name_or_id()
            ))
            self.xes_trace.add_event(e)

            RunInstance.transition_fired(
                    self, process_id, time, transition_id, values)

        def event_send(self, process_id, time, target_id, size, edge_id):
            assert isinstance(self.last_event_activity, TransitionFire)
            transition_fire = self.last_event_activity

            e = xes.Event()
            e.add_attribute(xes.Attribute(
                type="int",
                key="timestamp",
                value=str(time),
            ))
            e.add_attribute(xes.Attribute(
                type="int",
                key="trans-id",
                value=str(transition_fire.transition.id)
            ))
            e.add_attribute(xes.Attribute(
                type="int",
                key="pid",
                value=str(process_id)
            ))
            e.add_attribute(xes.Attribute(
                type="string",
                key="activity",
                value="send/{0}/{1}".format(process_id, target_id)
            ))

            # optional attributes
            e.add_attribute(xes.Attribute(
                type="int",
                key="tpid",
                value=str(target_id)
            ))
            e.add_attribute(xes.Attribute(
                type="int",
                key="msg-size",
                value=str(size)
            ))
            self.xes_trace.add_event(e)

            RunInstance.event_send(
                    self, process_id, time, target_id, size, edge_id)

        def event_receive(self, process_id, time, origin_id):
            # take the first packet from the queue
            # TODO: how to solve receive problem, there is no posibility to
            #       say exactly which transition will receive this
            #packet = self.packets[process_id * self.process_count + origin_id][0]
            #print self.net.item_by_id(packet.edge_id).to_item

            e = xes.Event()
            e.add_attribute(xes.Attribute(
                type="int",
                key="timestamp",
                value=str(time)
            ))
            #e.add_attribute(xes.Attribute( # TODO: how to get this?
                #type="int",
                #key="trans-id",
                #value=str(transition_fire.transition.id)
            #))
            e.add_attribute(xes.Attribute(
                type="int",
                key="pid",
                value=str(process_id)
            ))
            e.add_attribute(xes.Attribute(
                type="string",
                key="activity",
                value="receive/{0}/{1}".format(origin_id, process_id)
            ))

            # optional attributes
            e.add_attribute(xes.Attribute(
                type="int",
                key="opid",
                value=str(origin_id)
            ))
            self.xes_trace.add_event(e)

            RunInstance.event_receive(self, process_id, time, origin_id)

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
            log.add_classifier(xes.Classifier("activity_uid",
                                              "trans_id pid activity"))

        return extensions.Source("Tracelog for process mining",
                                 datatypes.t_xes,
                                 log)

extensions.add_operation(TracelogToXES)
