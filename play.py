#!/usr/bin/env python3
import os
import subprocess
import configparser
import time
import gzip

mpvpath1 = '/usr/bin/mpv'
mpvpath2 = '/usr/local/bin/mpv'
mpvpath3 = './mpv'
hassound = True
if not os.path.exists(mpvpath1) and not os.path.exists(mpvpath2) and not os.path.exists(mpvpath3):
    print("脚本依赖应用程序mpv，如果需要播放声音，请安装mpv后再运行该脚本")
    hassound = False
null = open('/dev/null', 'w')
parser = configparser.ConfigParser()
parser.read(os.path.join(os.path.dirname(__file__),'./settings.conf'))
fps = parser['General']['fps']
interval = 1 / (int(fps))
width = parser['General']['width']
height = parser['General']['height']
# print(fps, width, height)
print("请将终端大小调至%sx%s,并按回车键继续" % (width, int(height) + 1))
input()
os.system('clear')
if hassound:
    sound = subprocess.Popen(['mpv', './sound.aac'], stdout=null, stderr=null)
firsttime = time.time()
while True:
    os.system('clear')
    currenttime = time.time()
    idx = int((currenttime - firsttime) / interval)
    filepath = './char_%09d.txt.gz' % idx
    if  os.path.exists(filepath):
        with gzip.open(filepath,'rb') as f:
            print(f.read().decode('utf-8'))
    else:
        break
    time.sleep(0.01)
