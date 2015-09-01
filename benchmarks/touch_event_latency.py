# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#
# Copyright (C) 2015 Canonical Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

from mir_perf_framework import PerformanceTest, Server, Client
import time
import statistics
import shutil
import sys
from common import get_pointing_device
import report_types

####### TEST #######


def perform_test():
    host = Server(reports=["input"])
    nested = Server(executable=shutil.which("qtmir-demo-shell"),
                    host=host,
                    reports=["input","client-input-receiver"],
                    env={"QT_QPA_PLATFORM": "mirserver", "QML_NO_TOUCH_COMPRESSION": "1"})
    client = Client(executable=shutil.which("qtmir-demo-client"),
                    server=nested,
                    reports=["input","client-input-receiver"],
                    env={"QT_QPA_PLATFORM": "ubuntumirclient", "QML_NO_TOUCH_COMPRESSION": "1"},
                    options=["--", "--desktop_file_hint=/usr/share/applications/qtmir-demo-client.desktop"])

    test = PerformanceTest([host, nested, client])
    test.start()

    results = report_types.Results()
    processes = report_types.Processes()
    processes.add_child(report_types.Process("Host", host.process.pid))
    processes.add_child(report_types.Process("Nested Server", nested.process.pid))
    processes.add_child(report_types.Process("Client", client.process.pid))
    results.add_child(processes)

    time.sleep(3) # wait for settle

    host_pid = host.process.pid
    nested_pid = nested.process.pid
    client_pid = client.process.pid

    touch = get_pointing_device()
    time.sleep(1) # let mir pick up the new input device
    touch.drag(100, 100, 1000, 100, 5, 0.006)
    touch.drag(1000, 100, 1000, 1000, 5, 0.006)
    touch.drag(1000, 1000, 100, 1000, 5, 0.006)
    touch.drag(100, 1000, 100, 100, 5, 0.006)

    # time.sleep(5) # wait for settle
    time.sleep(2) # wait for settle
    test.stop()

    ####### TRACE PARSING #######

    trace = test.babeltrace()

    server_touch_data_timestamps = {}
    client_touch_data_timestamps = {}
    client_touch_data_latency = {}

    qtmir_touch_dispatch_start = {}
    qtmir_touch_dispatch_end = {}
    qtmir_touch_consume_start = {}
    qtmir_touch_consume_end = {}

    events = report_types.Events()

    for event in trace.events:
        events.add_child(report_types.Event(event))
        pid = event["vpid"]
        if event.name == "mir_client_input_receiver:touch_event":
            if pid not in client_touch_data_timestamps: client_touch_data_timestamps[pid] = []
            if pid not in client_touch_data_latency: client_touch_data_latency[pid] = []
            diff = (event.timestamp - event["event_time"]) / 1000000.0
            if diff > 0:
                client_touch_data_timestamps[pid].append(event.timestamp)
                client_touch_data_latency[pid].append(diff)

        elif event.name == "mir_server_input:published_motion_event":
            if pid not in server_touch_data_timestamps: server_touch_data_timestamps[pid] = []
            server_touch_data_timestamps[pid].append(event["event_time"])

        elif event.name == "qtmirserver:touchEventDisptach_start":
            if pid not in qtmir_touch_dispatch_start: qtmir_touch_dispatch_start[pid] = []
            qtmir_touch_dispatch_start[pid].append(event.timestamp)

        elif event.name == "qtmirserver:touchEventDisptach_end":
            if pid not in qtmir_touch_dispatch_end: qtmir_touch_dispatch_end[pid] = []
            qtmir_touch_dispatch_end[pid].append(event.timestamp)

        elif event.name == "qtmir:touchEventConsume_start":
            if pid not in qtmir_touch_consume_start: qtmir_touch_consume_start[pid] = []
            qtmir_touch_consume_start[pid].append(event.timestamp)

        elif event.name == "qtmir:touchEventConsume_end":
            if pid not in qtmir_touch_consume_end: qtmir_touch_consume_end[pid] = []
            qtmir_touch_consume_end[pid].append(event.timestamp)

    # LATENCY MEANS

    if nested_pid in client_touch_data_latency:
        nested_data = client_touch_data_latency[nested_pid]
        nested_latency_xml = report_types.ResultsData(
            "nested_latency",
            statistics.mean(nested_data),
            statistics.stdev(nested_data),
            "Kernel to nested server latency")
        for value in nested_data:
            nested_latency_xml.add_data(value)
        results.add_child(nested_latency_xml)
        nested_latency_xml.generate_histogram("nested_latency")
    else:
        results.add_child(report_types.Error("No nested server touch latency data"))

    if client_pid in client_touch_data_latency:
        client_data = client_touch_data_latency[client_pid]

        client_latency_xml = report_types.ResultsData(
            "client_latency",
            statistics.mean(client_data),
            statistics.stdev(client_data),
            "Kernel to client latency")
        for value in client_data:
            client_latency_xml.add_data(value)
        results.add_child(client_latency_xml)
        client_latency_xml.generate_histogram("client_latency")
    else:
        results.add_child(report_types.Error("No client touch latency data"))

    # EVENT RATES
    if host_pid in server_touch_data_timestamps:
        last_timestamp = -1
        input_rate = []

        for next_timestamp in server_touch_data_timestamps[host_pid]:
            if last_timestamp != -1:
                diff = (next_timestamp - last_timestamp) / 1000000.0
                input_rate.append(diff)
            last_timestamp = next_timestamp

        input_rate_xml = report_types.ResultsData(
            "host_input_ate",
            statistics.mean(input_rate),
            statistics.stdev(input_rate),
            "Host input event rate")
        for value in input_rate:
            input_rate_xml.add_data(value)
        results.add_child(input_rate_xml)
        input_rate_xml.generate_histogram("host_input_rate")
    else:
        results.add_child(report_types.Error("No host server input event timestamp data"))

    if nested_pid in client_touch_data_timestamps:
        last_timestamp = -1
        input_rate = []
        for next_timestamp in client_touch_data_timestamps[nested_pid]:
            if last_timestamp != -1:
                diff = (next_timestamp - last_timestamp) / 1000000.0
                input_rate.append(diff)
            last_timestamp = next_timestamp

        input_rate_xml = report_types.ResultsData(
            "nested_input_rate",
            statistics.mean(input_rate),
            statistics.stdev(input_rate),
            "Nested server event rate")
        for value in input_rate:
            input_rate_xml.add_data(value)
        results.add_child(input_rate_xml)
        input_rate_xml.generate_histogram("nested_input_rate")
    else:
        results.add_child(report_types.Error("No nested server input event timestamp data"))

    if client_pid in client_touch_data_timestamps:
        last_timestamp = -1
        input_rate = []
        for next_timestamp in client_touch_data_timestamps[client_pid]:
            if last_timestamp != -1:
                diff = (next_timestamp - last_timestamp) / 1000000.0
                input_rate.append(diff)
            last_timestamp = next_timestamp

        input_rate_xml = report_types.ResultsData(
            "client_input_rate",
            statistics.mean(input_rate),
            statistics.stdev(input_rate),
            "Client event rate")
        for value in input_rate:
            input_rate_xml.add_data(value)
        results.add_child(input_rate_xml)
        input_rate_xml.generate_histogram("client_input_rate")
    else:
        results.add_child(report_types.Error("No client event timestamp data"))

    qtmir_loop_data = []
    dispatch_data = []
    consume_data = []

    # TIME BETWEEN TRACEPOINTS
    dispatch_starts = qtmir_touch_dispatch_start[nested_pid] if nested_pid in qtmir_touch_dispatch_start else []
    dispatch_ends = qtmir_touch_dispatch_end[nested_pid] if nested_pid in qtmir_touch_dispatch_end else []
    consume_starts = qtmir_touch_consume_start[nested_pid] if nested_pid in qtmir_touch_consume_start else []
    consume_ends = qtmir_touch_consume_end[nested_pid] if nested_pid in qtmir_touch_consume_end else []

    # since there's no uniqueness to events, we need to assume all events are 1:1 through system
    if len(dispatch_starts) > 0 and len(dispatch_starts) == len(dispatch_ends) and len(dispatch_starts) == len(consume_starts) and len(consume_starts) == len(consume_ends):
        i = 0

        for start in dispatch_starts:
            dispatch_diff = (dispatch_ends[i] - start) / 1000000.0
            consume_diff = (consume_ends[i] - consume_starts[i]) / 1000000.0
            loop_dif = (consume_starts[i] - dispatch_ends[i]) / 1000000.0
            dispatch_data.append(dispatch_diff);
            consume_data.append(consume_diff);
            qtmir_loop_data.append(loop_dif);
            i=i+1

        qtmir_loop_xml = report_types.ResultsData(
            "qtmir_eventloop",
            statistics.mean(qtmir_loop_data),
            statistics.stdev(qtmir_loop_data),
            "Time spent in qtmir event loop")
        for value in qtmir_loop_data:
            qtmir_loop_xml.add_data(value)
        results.add_child(qtmir_loop_xml)
        qtmir_loop_xml.generate_histogram("qtmir_eventloop")

        qtmir_dispatch_xml = report_types.ResultsData(
            "qtmir_dispatch",
            statistics.mean(dispatch_data),
            statistics.stdev(dispatch_data),
            "Time QteventFeeder spent dispatching event to qt")
        for value in dispatch_data:
            qtmir_dispatch_xml.add_data(value)
        results.add_child(qtmir_dispatch_xml)
        qtmir_dispatch_xml.generate_histogram("qtmir_dispatch")

        qtmir_consume_xml = report_types.ResultsData(
            "qtmir_consume",
            statistics.mean(consume_data),
            statistics.stdev(consume_data),
            "Time MirSurfaceItem spent consiming event")
        for value in consume_data:
            qtmir_consume_xml.add_data(value)
        results.add_child(qtmir_consume_xml)
        qtmir_consume_xml.generate_histogram("qtmir_consume")

    else:
        results.add_child(report_types.Error("Cannot calculate QtMir loop data - Dispatch event count did not match surface consume event count"))

    results.add_child(events)
    return results

if __name__ == "__main__":
    results = perform_test();
    f = open("touch_event_latency.xml", "w")
    f.write(results.to_string())
