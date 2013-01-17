#!/usr/bin/env python
# Nicolas Seriot, 2013-01-17

"""
UniBinary, or "Base64 for Unicode".
Encodes and decodes binary data into Unichode characters.
The encoded text can be copied / pasted / posted on Twitter and stored in UTF-16 encoded files.
http://github.com/nst/UniBinary/

$ python unibinary.py -h

$ python unibinary.py -e /bin/date > /tmp/date.txt
$ file /tmp/date.txt 
/tmp/date.txt: UTF-8 Unicode text, with very long lines, with no line terminators

$ python unibinary.py -d /tmp/date.txt > /tmp/date
$ file /tmp/date
/tmp/date: Mach-O 64-bit executable x86_64

$ chmod +x /tmp/date
$ /tmp/date
Thu Jan 17 18:02:24 CET 2013
"""

import struct
import sys
import unittest
import codecs
import hashlib
import argparse

__author__ = "Nicolas Seriot"
__license__ = "BSD"
__status__ = "Draft"

# http://docs.python.org/2/howto/unicode.html
# http://unicode.org/Public/UNIDATA/Blocks.txt
U12_start, U12_length = (0x4E00, 0x1000) # CJK Unified Ideographs (subset) - encodes 12 bits values
U8_start, U8_length = (0x0400, 0x100)    # Cyrillic                        - encodes 8 bits values

def two_unichr_to_repeat_byte_ntimes(b, n):
    if n > 0xFFF:
        raise ValueError
    
    if b > 0xFF:
        raise ValueError
    
    uni_b = unichr(U8_start + b)
    uni_r = unichr(U12_start + n)
    
    return (uni_b, uni_r)

def unichr_08_from_int(i):
    if i > (U8_start + U8_length):
        print "-- unichr_08_from_int: 0x%x" % i
        raise ValueError
    
    return unichr(U8_start + i)

def unichr_12_from_int(i):
    if i > (U12_start + U12_length):
        print "-- unichr_12_from_int: 0x%x" % i
        raise ValueError
    
    return unichr(U12_start + i)

def int_from_unichr_08(u):
    
    i = ord(u)
    
    if i < U8_start or i > (U8_start + U8_length):
        print "-- int_from_unichr_08: %c" % u
        raise ValueError

    return i - U8_start

def int_from_unichr_12(u):
    
    i = ord(u)
    
    if i < U12_start or i > (U12_start + U12_length):
        print "-- int_from_unichr_12: %c" % u
        raise ValueError

    return i - U12_start

def two_twelve_bits_values_from_three_bytes(a, b, c):
    # (0x12, 0x34, 0x56) -> (0x123, 0x456)

    if a > 0xFF or b > 0xFF or c > 0xFF:
        raise ValueError

    s1 = (a << 4) + (b >> 4)
    s2 = ((b & 0xF) << 8) + c
    
    return (s1, s2)

def three_bytes_from_two_twelve_bits_values(i1, i2):
    # (0x123, 0x456) -> (0x12, 0x34, 0x56)
    
    if i1 > 0xFFF or i2 > 0xFFF:
        raise ValueError
    
    b1 = i1 >> 4
    b2 = ((i1 & 0xF) << 4) + ((i2 & 0xF00) >> 8)
    b3 = i2 & 0x0FF
    
    return (b1, b2, b3)
    
def number_of_left_instances_from_index(l, index):
    i = index
    c = 0
    x = l[i]
    while i < len(l):
        if l[i] == x:
            c += 1
        else:
            break
        i += 1
    
    return c

def three_bytes_from_unichars(u1, u2):
    i1 = int_from_unichr_12(u1)
    i2 = int_from_unichr_12(u2)
    (b1, b2, b3) = three_bytes_from_two_twelve_bits_values(i1, i2)
    return (b1, b2, b3)

def repeated_bytes_from_unichars(u1, u2):
    b = int_from_unichr_08(u1)
    n = int_from_unichr_12(u2)
    return ([b])*n

def two_bytes_from_unichars(u1, u2):
    b1 = int_from_unichr_08(u1)
    b2 = int_from_unichr_08(u2)
    return (b1, b2)

def is_in_U8(u):
    i = ord(u)
    return i >= U8_start and i < (U8_start + U8_length)

def is_in_U12(u):
    i = ord(u)
    return i >= U12_start and i < (U12_start + U12_length)

def bytes_from_u1_u2(u1, u2):
    
    u1_in_U12 = is_in_U12(u1)
    u2_in_U12 = is_in_U12(u2)
    
    u1_in_U8 = is_in_U8(u1)
    u2_in_U8 = is_in_U8(u2)
    
    if u1_in_U12 and u2_in_U12:
        return three_bytes_from_unichars(u1, u2)
    elif u1_in_U8 and u2_in_U12:
        return repeated_bytes_from_unichars(u1, u2)
    elif u1_in_U8 and u2_in_U8:
        return two_bytes_from_unichars(u1, u2)
    else:
        print "-- %c %c 0x%x 0x%x" % (u1, u2, ord(u1), ord(u2))
        raise ValueError

def gen_encode_unichars_from_bytes(bytes):
    
    i = 0
    
    while (i < len(bytes)):
        r = number_of_left_instances_from_index(bytes, i)
        
        r = r % 0x1000
                
        if r > 3: # because 3 bytes get encoded as 2 unichars anyway
            # read N bytes | N > 3 and N < 0x1000, encode as 2 unichar
            length = r
            (uni_b, uni_n) = two_unichr_to_repeat_byte_ntimes(ord(bytes[i]), r)
            i += length
            yield (uni_b, uni_n)
        
        else:
            
            three_bytes_available = (len(bytes) - i) > 2
            
            if three_bytes_available:
                # read 3 bytes, encode 2 unichars 

                b = struct.unpack("BBB", bytes[i:i+3])
                i += 3

                (a,b,c) = b
                (s1, s2) = two_twelve_bits_values_from_three_bytes(a, b, c)
                yield (unichr_12_from_int(s1), unichr_12_from_int(s2))
            
            else:
                # read 1 byte, encode 1 unichar

                b = struct.unpack("B", bytes[i])
                i += 1

                yield (unichr_08_from_int(b[0]))

def gen_decode_bytes_from_string(s):
    
    i = 0
        
    while (i < len(s)):
        
        if (i + 2) <= len(s):
            (u1, u2) = s[i:i+2]
            i += 2
            
            bytes = bytes_from_u1_u2(u1, u2)
            
            yield bytes
        else:
            u1 = s[i]
            i += 1
            
            b = int_from_unichr_08(u1)

            yield tuple([b])

# tests

def shasum(filename):
    m = hashlib.sha1()
    with open(filename,'rb') as f: 
        for chunk in iter(lambda: f.read(128*m.block_size), b''): 
             m.update(chunk)
    return m.hexdigest()

class TestUnidata(unittest.TestCase):

    #def setUp(self):
    #    pass
    
    def test_unichr_12_encoding_decoding(self):
    
        for i in [0x0, 0x1, 0xAB, 0x123, 0xABC, 0xF, 0xFF, 0xFFF]:
    
            u = unichr_12_from_int(i)
            self.assertNotEqual(i, u)
            
            i2 = int_from_unichr_12(u)
            self.assertEqual(i, i2)
    
    def test_3_to_2_bytes(self):

        (a, b) = two_twelve_bits_values_from_three_bytes(0x12, 0x34, 0x56)
        
        self.assertEqual(a, 0x123, "0x%x" % a)
        self.assertEqual(b, 0x456, "0x%x" % b)
    
    def test_2_to_3_bytes(self):
        
        (a,b,c) = three_bytes_from_two_twelve_bits_values(0x123, 0x456)

        self.assertEqual(a, 0x12, "0x%x" % a)
        self.assertEqual(b, 0x34, "0x%x" % b)
        self.assertEqual(c, 0x56, "0x%x" % c)
    
    def test_encode_bytes(self):
        bytes = "\xab\xcd\xef\xff"
        
        gen = gen_encode_unichars_from_bytes(bytes)
        
        (u1, u2) = gen.next()
        (u3) = gen.next()
        
        self.assertEqual(u1, unichr(U12_start + 0xABC))
        self.assertEqual(u2, unichr(U12_start + 0xDEF))
        self.assertEqual(u3, unichr(U8_start + 0xFF))
    
    def test_decode_unichars(self):
        
        u1 = unichr(U12_start + 0xABC)
        u2 = unichr(U12_start + 0xDEF)
        
        s = u''
        s += u1
        s += u2
        
        gen = gen_decode_bytes_from_string(s)
               
        (a,b,c) = gen.next()
        
        self.assertEqual(a, 0xAB)
        self.assertEqual(b, 0xCD)
        self.assertEqual(c, 0xEF)
    
    def test_is_in_U8(self):
        self.assertFalse(is_in_U8(u"\u03FF"))
        
        self.assertTrue(is_in_U8(u"\u0400"))
        self.assertTrue(is_in_U8(u"\u04FF"))

        self.assertFalse(is_in_U8(u"\u0500"))
        
    def test_encoding_decoding_utf16_file(self):
        
        src = "/usr/bin/true"
        tmp = "/tmp/true.txt"
        cpy = "/tmp/true"
        
        for e in ['utf-8', 'utf-16']:

            f = open(src, "rb")
            bytes = f.read()
            f.close()
        
            f = codecs.open(tmp, 'w', encoding=e)
            
            for unichars in gen_encode_unichars_from_bytes(bytes):
                for u in unichars:
                    f.write(u)
            
            f.close()
            
            ##
            
            f = codecs.open(tmp, 'r', encoding=e)
            s = f.read()
            f.close()
            
            f = open(cpy, 'wb')
            for bytes in gen_decode_bytes_from_string(s):
                for b in bytes:
                    buf = struct.pack("B", b)
                    f.write(buf)
            f.close()
            
            shasum_src = shasum(src)
            shasum_cpy = shasum(cpy)
            
            self.assertEqual(shasum_src, shasum_cpy)
    
    def test_repeats(self):
    
        l = [1,1,1,2,1]
    
        n = number_of_left_instances_from_index(l, 0)
        
        self.assertEqual(n, 3)

# profiling

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

# main

def print_decoded_string(s):

    for bytes in gen_decode_bytes_from_string(s):
        for b in bytes:
            buf = struct.pack("B", b)
            sys.stdout.write(buf)

if __name__ == '__main__':
    
    if len(sys.argv) == 2 and sys.argv[1] == 'test':
        #unittest.main()
        suite = unittest.TestLoader().loadTestsFromTestCase(TestUnidata)
        unittest.TextTestRunner(verbosity=2).run(suite)
    elif len(sys.argv) == 2 and sys.argv[1] == 'profile':
        import hotshot
        from hotshot import stats
    
        for f in [profile_encode_file, profile_decode_file]:
    
            prof = hotshot.Profile("hotshot_stats.prof")
            prof.runcall(f)
            prof.close()
                
            s = stats.load("hotshot_stats.prof")
            s.strip_dirs()
            s.sort_stats('time', 'calls')
            s.print_stats(20)
    else:
        parser = argparse.ArgumentParser(description='UniBinary encodes and decodes binary data into Unicode characters.')
        parser.add_argument('-e','--encode', help='utf-8 file to encode')
        parser.add_argument('-d','--decode', help='utf-8 file to decode')
        parser.add_argument('-s','--string', help='utf-8 string to decode')
        args = vars(parser.parse_args())
        
        if args['encode']:    
            f = open(args['encode'], "rb")
            bytes = f.read()
            f.close()
        
            UTF8Writer = codecs.getwriter('utf-8')
            sys.stdout = UTF8Writer(sys.stdout)
    
            for unichars in gen_encode_unichars_from_bytes(bytes):
                string = unicode(''.join(unichars))
                sys.stdout.write(string)
            
        elif args['decode']:
            f = codecs.open(args['decode'], "r", encoding='utf-8')
            s = f.read()
            f.close()

            print_decoded_string(s)            
        elif args['string']:
            print_decoded_string(args['string'].decode('utf-8'))
        else:
            parser.print_help()
        