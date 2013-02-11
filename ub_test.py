#!/usr/bin/env python
# Nicolas Seriot, 2013-01-17

"""
UniBinary tests

$ python ub_tests.py
"""

from unibinary import *
import unittest

def shasum(filename):
    m = hashlib.sha1()
    with open(filename,'rb') as f: 
        for chunk in iter(lambda: f.read(128*m.block_size), b''): 
             m.update(chunk)
    return m.hexdigest()

class TestUnidata(unittest.TestCase):

    #def setUp(self):
    #    pass

    #def tearDown(self):
    #    pass
    
    def test_unichr_12_encoding_decoding(self):
    
        for i in [0x0, 0x1, 0xAB, 0x123, 0xABC, 0xF, 0xFF, 0xFFF]:
    
            u = unichr_12_from_int(i)
            self.assertNotEqual(i, u)
            
            i2 = int_from_u12b(u)
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
        
        self.assertEqual(u1, unichr(U12b_start + 0xABC))
        self.assertEqual(u2, unichr(U12b_start + 0xDEF))
        self.assertEqual(u3, unichr(U8_start + 0xFF))
    
    def test_decode_unichars(self):
        
        u1 = unichr(U12b_start + 0xABC)
        u2 = unichr(U12b_start + 0xDEF)
        
        s = u''
        s += u1
        s += u2
        
        gen = gen_decode_bytes_from_string(s)
               
        (a,b,c) = gen.next()
        
        self.assertEqual(a, 0xAB)
        self.assertEqual(b, 0xCD)
        self.assertEqual(c, 0xEF)
    
    def test_is_in_U8b(self):
        self.assertFalse(is_in_U8b(u"\u03FF"))
        
        self.assertTrue(is_in_U8b(u"\u0400"))
        self.assertTrue(is_in_U8b(u"\u04FF"))

        self.assertFalse(is_in_U8b(u"\u0500"))
        
    def test_encoding_decoding_utf16_file(self):
        
        src = "/usr/bin/true"
        tmp = "/tmp/true.txt"
        cpy = "/tmp/true"
        
        import os
        if not os.path.exists(src):
            print "-- WARNING: cannot test %s, file does not exist" % src
            return
        
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

    def test_unichr_12a_from_two_ascii(self):
        u = unichr_12a_from_two_ascii('Z', 'E')
        self.assertEqual(u, u"\u9485")

        u = unichr_12a_from_two_ascii('z', ',')
        self.assertEqual(u, u"\u8CAC")

    def test_ascii_characters_encoding(self):
        s = "abc"
        
        gen = gen_encode_unichars_from_bytes(s)
        
        u0 = gen.next()
        u1 = gen.next()
        
        self.assertEqual(u0, u"\u9662")
        self.assertEqual(u1, u"\u0463")

    def test_ascii_characters_encoding_2(self):
    
        s = "ZE"
        
        gen = gen_encode_unichars_from_bytes(s)
        
        u0 = gen.next()
        
        self.assertEqual(u0, unichr_12a_from_two_ascii('Z', 'E'))
    
    def test_ascii_characters_decoding(self):
    
        s = [u"\u9662", u"\u0463"]
        
        s2 = []
        for chunks in gen_decode_bytes_from_string(s):
            for b in chunks:
                s2.append(b)

        self.assertEqual(s2[0], ord('a'))
        self.assertEqual(s2[1], ord('b'))
        self.assertEqual(s2[2], ord('c'))
    
    def test_ascii_characters_decoding_2(self):
    
        s = [u"\u9485"]
        
        s2 = []
        for chunks in gen_decode_bytes_from_string(s):
            for b in chunks:
                s2.append(b)
        
        self.assertEqual(s2[0], ord('Z'))
        self.assertEqual(s2[1], ord('E'))

    def test_five_bytes_encoding(self):
        bytes = "\xab\xcd\xef\xab\xcd"
        gen = gen_encode_unichars_from_bytes(bytes)
        
        (u1, u2) = gen.next()
        (u3) = gen.next()
        (u4) = gen.next()
        
        self.assertEqual(u1, unichr(U12b_start + 0xABC))
        self.assertEqual(u2, unichr(U12b_start + 0xDEF))
        self.assertEqual(u3, unichr_08_from_int(0xAB))
        self.assertEqual(u4, unichr_08_from_int(0xCD))
    
    def test_ascii_and_bytes_encoding(self):
        bytes = "\xab\xcd\xef"
        bytes += "\x61\x62\x63\x64\x65" # abcde
        
        gen = gen_encode_unichars_from_bytes(bytes)
        
        (u1, u2) = gen.next()
        (u3) = gen.next()
        (u4) = gen.next()
        (u5) = gen.next()
        
        self.assertEqual(u1, unichr(U12b_start + 0xABC))
        self.assertEqual(u2, unichr(U12b_start + 0xDEF))
        self.assertEqual(u3, unichr_12a_from_two_ascii('a', 'b'))
        self.assertEqual(u4, unichr_12a_from_two_ascii('c', 'd'))
        self.assertEqual(u5, unichr_08_from_int(ord('e')))
    
    def test_ascii_and_bytes_decoding(self):
        
        u1 = unichr(U12b_start + 0xABC)
        u2 = unichr(U12b_start + 0xDEF)
        u3 = unichr_12a_from_two_ascii('a', 'b')
        u4 = unichr_12a_from_two_ascii('c', 'd')
        u5 = unichr_08_from_int(ord('e'))
                
        s = u''
        s += u1
        s += u2
        s += u3
        s += u4
        s += u5
        
        gen = gen_decode_bytes_from_string(s)
               
        (a,b,c) = gen.next()
        (d,e) = gen.next()
        (f,g) = gen.next()
        (h) = gen.next()
                
        self.assertEqual(a, 0xAB)
        self.assertEqual(b, 0xCD)
        self.assertEqual(c, 0xEF)
            
    def test_repeats(self):
    
        l = [1,1,1,2,1]
    
        n = number_of_left_instances_from_index(l, 0)
        
        self.assertEqual(n, 3)

    def test_repeats_2(self):
    
        bytes = "\xAB\xCD\xEF\xFF\xFF\xFF\xFF\x00"
        
        gen = gen_encode_unichars_from_bytes(bytes)
        
        (u1, u2) = gen.next()
        (u3, u4) = gen.next()
        (u5) = gen.next()
    
    def test_ascii_text_encoding_decoding(self):
    
        s = "if I'd listened everything that they said to me, took the time to bleed from all the tiny little arrows shot my way, I wouldn't be here! the ones who don't do anything are always the ones who try to put you down. I'm talking to you: hero time starts right now! time to shine!"
        
        encode_gen = gen_encode_unichars_from_bytes(s)

        e = [b for b in encode_gen]

        s2 = ''.join([chr(c) for chunk in gen_decode_bytes_from_string(e) for c in chunk])
        
        self.assertEqual(s, s2)

    def test_ascii_text_encoding_decoding_2(self):
    
        s = ''.join([chr(i) for i in range(32, 128)])

        encode_gen = gen_encode_unichars_from_bytes(s)

        e = [b for b in encode_gen]

        self.assertTrue(len(e) * 2 == len(s))

        s2 = ''.join([chr(c) for chunk in gen_decode_bytes_from_string(e) for c in chunk])
        
        self.assertEqual(s, s2)

if __name__ == '__main__':
#    unittest.main()
    suite = unittest.TestLoader().loadTestsFromTestCase(TestUnidata)
    unittest.TextTestRunner(verbosity=2).run(suite)
