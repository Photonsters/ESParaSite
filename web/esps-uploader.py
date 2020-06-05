"""
ESParaSite - GUI Upoloader v0.1

        Authors: Andy  (SolidSt8Dad)Eakin

        Please see /VERSION for Hstory

        All Derived Content is subject to the most restrictive licence of it's
        source.

        All Original content is free and unencumbered software released into the
        public domain.

        The Author(s) are extremely grateful for the amazing open source
        communities that work to support all of the sensors, microcontrollers,
        web standards, etc.
"""

import os
import requests

from os import listdir
from os.path import isfile, join

espsMdns = input("Enter mDNS name of ESParaSite: ")

filesDir = os.path.join(os.getcwd(), "gui")

fileList = [f for f in listdir(filesDir) if isfile(join(filesDir, f))]

uploadUrl = "http://" + espsMdns + "/upload"

for file in fileList:
    filePath = os.path.join(filesDir, file)
    with open(filePath, 'rb') as f:
        r = requests.post(uploadUrl, files={filePath: f})
