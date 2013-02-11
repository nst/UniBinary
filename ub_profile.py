#!/usr/bin/env python
# Nicolas Seriot, 2013-01-17

"""
UniBinary profiling

$ python ub_profile.py
"""

from unibinary import *
import hotshot
from hotshot import stats

def profile_encode_file():
    #f = open("/usr/bin/true", "rb")
    f = open("/Users/nst/Desktop/sc.png", "rb") # any file ~ 800 KB
    bytes = f.read()
    f.close()
    
    f = codecs.open("/tmp/tmp.txt", 'w', encoding='utf-16')
    for unichars in gen_encode_unichars_from_bytes(bytes):
        for u in unichars:
            f.write(u)
    f.close()

def profile_decode_file():
    f = codecs.open("/tmp/tmp.txt", "r", encoding='utf-16')
    s = f.read()
    f.close()
    
    f = open("/tmp/tmp.bin", 'wb')
    for chunk in gen_decode_bytes_from_string(s):
        for b in chunk:
            buf = struct.pack("B", b)
            f.write(buf)
    f.close()

if __name__ == '__main__':

    for f in [profile_encode_file, profile_decode_file]:
    
        prof = hotshot.Profile("hotshot_stats.prof")
        prof.runcall(f)
        prof.close()
        
        s = stats.load("hotshot_stats.prof")
        s.strip_dirs()
        s.sort_stats('time', 'calls')
        s.print_stats(20)
