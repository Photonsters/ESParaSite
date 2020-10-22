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
import math
import sys
import getopt

from os import listdir
from os.path import isfile, join
from datetime import datetime, timezone
from typing import List, Tuple

USAGE = f"Usage: python {sys.argv[0]} [--help] | [-i <IP Address>] -or- [-m <mDNS Name>][--upload][--setclock]"
VERSION = f"{sys.argv[0]} version 1.0.0"

def main() -> None:
    args = sys.argv[1:]
    if not args:
        raise SystemExit(USAGE)
    ipaddr, mdnsName, setClock, upload = parse(args)
    if ipaddr:
        print("IP Address " + ipaddr)
        target = ipaddr
    elif mdnsName:
        print("MDNS Name " + mdnsName)
        target = mdnsName
    if upload:
        upload(target)
    if setClock:
        setclock(target)

def parse(args: List[str]) -> Tuple[str, List[int]]:
    options, arguments = getopt.getopt(
        args,                                                   # Arguments
        'vhi:m:',                                               # Short option definitions
        ["version", "help", "ipaddr=", "mdns=", "setclock"])    # Long option definitions
    ipaddr = ""
    mdnsName = ""
    setClock = False
    upload = False
    for o, a in options:
        if o in ("-v", "--version"):
            print(VERSION)
            sys.exit()
        if o in ("-h", "--help"):
            print(USAGE)
            sys.exit()
        if o in ("-i", "--ipaddr"):
            ipaddr = a
        if o in ("-m", "--mdns"):
            mdnsName = a
           #upload(mdnsName)
        if o in ("--setclock"):
            setClock = True
        if o in ("--upload"):
            upload = True
    if not options or len(arguments) > 3:
        raise SystemExit(USAGE)
    try:
        operands = [int(arg) for arg in arguments]
    except:
        raise SystemExit(USAGE)
    return ipaddr, mdnsName, setClock, upload


def upload(espsAddr):

    filesDir = os.path.join(os.getcwd(), "gui")

    fileList = [f for f in listdir(filesDir) if isfile(join(filesDir, f))]

    uploadUrl = "http://" + espsAddr + "/upload"

    for file in fileList:
        filePath = os.path.join(filesDir, file)
        print("Uploading " + file)
        with open(filePath, 'rb') as f:
            r = requests.post(uploadUrl, files={filePath: f})


def setclock(espsAddr):

    # seconds from epoch:
    now_utc = datetime.now(timezone.utc).timestamp()
    now_utc_sec = math.floor(now_utc)
    print("Setting RTC to UTC epoch: ")

    uploadUrl = "http://" + espsAddr + "/api?setRtcClock="
    payload = {'timestamp': now_utc_sec}
    print(payload)
    r = requests.post(uploadUrl, params=payload)


if __name__ == "__main__":
    main()
