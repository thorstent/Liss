#/usr/bin/python3

import glob
import os
import string

def output_after_colon(content, start, string1):
  start2 = content.find(string1, start)
  start3 = content.find(':', start2)
  end = content.find('\n', start3)
  
  print(content[start3+2: end], end="")

def process_statistic(file, name):
  f = open(file, 'r')
  print(os.path.basename(file)[:-4].replace("_","\\_"), end=" & ")
  content = f.read()
  start = content.find(name)
  if start != -1 and content.find('Found no valid lock placement') == -1:
      output_after_colon(content, start, 'locks used')
      print("", end=" & ")
      output_after_colon(content, start, 'lock operations')
      print("", end=" & ")
      output_after_colon(content, start, 'unlock operations')
      print("", end=" & ")
      output_after_colon(content, start, 'abstract instructions inside a lock')
      print("", end=" & ")
      output_after_colon(content, start, 'Time for this')
  
  
  print('\\\\')

def process_files(files):
  print("absmin")
  for f in files:
    process_statistic(f, 'absmin')
  print("\nsmall")
  for f in files:
    process_statistic(f, 'small')
  print("\ncoarse")
  for f in files:
    process_statistic(f, 'coarse')
  print("\nunopt")
  for f in files:
    process_statistic(f, 'unopt')


files1 = glob.glob("tests/cav13/*.log")
files1.sort()
files2 = glob.glob("tests/cav14/*.log")
files2.sort()
files3 = glob.glob("tests/linux_drivers/*.log")
files3.sort()

files = files1 + files2 + files3

process_files(files)
