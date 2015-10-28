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

import subprocess
import os
import shutil

class Node:
    def __init__(self, name):
        self.name = name
        self.children = []
    
    def add_child(self, child):
        self.children.append(child)

    def to_string(self):
        output = "<%s>" % self.name
        for child in self.children:
            output += child.to_string()
        output += "</%s>" % self.name
        return output

class Results(Node):
    def __init__(self):
        super().__init__("results")

class Events(Node):
    def __init__(self):
        super().__init__("events")

class Event:
    def __init__(self, event):
        self.pid = event["vpid"]
        self.name = event.name
        self.timestamp = event.timestamp

    def to_string(self):
        output = "<event "
        output += "pid='{}' ".format(self.pid)
        output += "name='{}' ".format(self.name)
        output += "timestamp='{}' ".format(self.timestamp)
        output += "/>"
        return output

class Processes(Node):
    def __init__(self):
        super().__init__("processes")

class Process:
    def __init__(self, name, pid):
        self.name = name
        self.pid = pid

    def to_string(self):
        output = "<process "
        output += "name='{}' ".format(self.name)
        output += "pid='{}' ".format(self.pid)
        output += "/>"
        return output

class ResultsData:
    def __init__(self, name, mean=0.0, deviation=0.0, comment=""):
        self.data = []
        self.name = name
        self.mean = mean
        self.deviation = deviation
        self.comment = comment

    def add_data(self, value):
        self.data.append(value)

    def to_string(self):
        output = "<data name='{}' mean='{}' deviation='{}' comment='{}' count='{}'>".format(
            self.name,
            self.mean,
            self.deviation, 
            self.comment, 
            len(self.data))
        output += "<values>"
        output += ",".join(map( lambda x: str(x), self.data))
        output += "</values>"
        output += "</data>"
        return output;

    def generate_histogram(self, filename):
        cmdline = [
            shutil.which("Rscript"),
            os.path.split(os.path.abspath(__file__))[0] + "/touch_event_latency.R",
            "data.csv",
            "%s.png" % filename]
        # Use R to generate a histogram plot
        f = open("data.csv", "w")
        f.write(",".join(map( lambda x: str(x), self.data)));
        f.close()
        process = subprocess.Popen(cmdline)
        process.wait()
        if process.returncode != 0:
            print("Failed to generate histogram");
        os.remove("data.csv")


class Error:    
    def __init__(self, comment):
        self.comment = comment

    def to_string(self):
        return "<error comment='{}' />".format(self.comment)
        return output