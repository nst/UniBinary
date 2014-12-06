/**
 * UniBinary - Encodes and decodes data into printable UniCode characters.
 *
 * Authors:
 * Nicolas Seriot, 2013-01-17
 * Toolsley, 2014-12-03 (JavaScript port)
 *
 * License: BSD
 *
 */
(function (root, factory) {
    if (typeof define === 'function' && define.amd) {
        define([], factory);
    } else if (typeof exports === 'object') {
        module.exports = factory();
    } else {
        root.unibinary = factory();
  }
}(this, function () {

//encodes ascii characters (7 bits)
    var U12a_0_0_start = 0x5E00; // CJK Unified Ideographs (subset) - encodes 12 bits (2 ascii) - MSB 0,0
    var U12a_0_1_start = 0x6E00; // CJK Unified Ideographs (subset) - encodes 12 bits (2 ascii) - MSB 0,1
    var U12a_1_0_start = 0x7E00; // CJK Unified Ideographs (subset) - encodes 12 bits (2 ascii) - MSB 1,0
    var U12a_1_1_start = 0x8E00; // CJK Unified Ideographs (subset) - encodes 12 bits (2 ascii) - MSB 1,1
    var U12a_length = 0x1000;

//encodes arbitrary bits
    var U12b_start = 0x4E00; // CJK Unified Ideographs (subset) - encodes 12 bits
    var U12b_length = 0x1000;
    var U8_start = 0x0400;   // Cyrillic                        - encodes 8 bits
    var U8_length = 0x100;

    var two_unichr_to_repeat_byte_ntimes = function (b, n) {

        if (n > 0xFFF) throw new Error("ValueError");

        if (b > 0xFF) throw new Error("ValueError");

        var uni_b = String.fromCharCode(U8_start + b);
        var uni_r = String.fromCharCode(U12b_start + n);

        return uni_b + uni_r;

    }

    var unichr_12a_from_two_ascii = function (a1, a2) {

        var i1 = a1;
        var i2 = a2;

        var unicode_start = null;

        if ((i1 < 64) && (i2 < 64)) {
            unicode_start = U12a_0_0_start;
        } else if ((i1 < 64) && (i2 >= 64)) {
            i2 -= 64;
            unicode_start = U12a_0_1_start;
        } else if ((i1 >= 64) && (i2 < 64)) {
            i1 -= 64;
            unicode_start = U12a_1_0_start;
        } else if ((i1 >= 64) && (i2 >= 64)) {
            i1 -= 64;
            i2 -= 64;
            unicode_start = U12a_1_1_start;
        }

        return String.fromCharCode(unicode_start + (i1 << 6) + i2)
    }

    var unichr_08_from_int = function (i) {
        if (i > (U8_start + U8_length)) {
            console.error("-- unichr_08_from_int: 0x" + i.toString(16));
            throw new Error("ValueError");
        }

        return String.fromCharCode(U8_start + i);
    }

    var unichr_12_from_int = function (i) {
        if (i > (U12b_start + U12b_length)) {
            console.error("-- unichr_12_from_int: 0x" + i.toString(16));
            throw new Error("ValueError");
        }

        return String.fromCharCode(U12b_start + i);
    }

    var int_from_u08b = function (u) {
        i = u.charCodeAt(0);
        if ((i < U8_start) || (i > (U8_start + U8_length))) {
            console.error("-- int_from_u8: " + u.toString());
            throw new Error("ValueError");
        }

        return i - U8_start;
    }

    var two_bytes_from_u12a = function (u) {
        var i1 = null;
        var i2 = null;
        var unicode_start = null;
        i = u.charCodeAt(0);

        for (var j = 0, start; start = [U12a_0_0_start, U12a_0_1_start, U12a_1_0_start, U12a_1_1_start][j]; j++) {
            if ((i >= start) && (i < (start + U12a_length)))
                unicode_start = start;
        }

        if (!unicode_start) {
            console.error("-- two_bytes_from_u12a ord=0x" + u.charCodeAt(0));
            throw new Error("ValueError");
        }

        var value = i - unicode_start;
        var b0 = (value & 0xFC0) >> 6;
        var b1 = i & 0x3F;

        switch (unicode_start) {
            case U12a_0_1_start:
                b1 += 64;
                break;
            case U12a_1_0_start:
                b0 += 64;
                break;
            case U12a_1_1_start:
                b0 += 64;
                b1 += 64;
        }

        return [b0, b1]

    }

    var int_from_u12b = function (u) {
        var i = u.charCodeAt(0);
        if ((i < U12b_start) || ( i > (U12b_start + U12b_length))) {
            console.error("-- int_from_u12b: " + u);
            throw new Error("ValueError");
        }

        return i - U12b_start;
    }


    var two_twelve_bits_values_from_three_bytes = function (a, b, c) {
        // (0x12, 0x34, 0x56) -> (0x123, 0x456)
        if ((a > 0xFF) || (b > 0xFF) || (c > 0xFF))
            throw new Error("ValueError");

        var s1 = (a << 4) + (b >> 4);
        var s2 = ((b & 0xF) << 8) + c;

        return [s1 , s2];
    }


    var three_bytes_from_two_twelve_bits_values = function (i1, i2) {
        // (0x123, 0x456) -> (0x12, 0x34, 0x56)
        if ((i1 > 0xFFF) || (i2 > 0xFFF))
            throw new Error("ValueError");

        var b1 = i1 >> 4;
        var b2 = ((i1 & 0xF) << 4) + ((i2 & 0xF00) >> 8);
        var b3 = i2 & 0x0FF;

        return [b1, b2, b3];

    }

    var number_of_left_instances_from_index = function (l, index) {
        var i = index;
        var c = 0;
        var x = l[i];

        while (i < l.length) {
            if (l[i] == x) {
                c += 1;
            } else {
                break;
            }
            i += 1;
        }
        return c;

    }

    var three_bytes_from_unichars = function (u1, u2) {
        var i1 = int_from_u12b(u1);
        var i2 = int_from_u12b(u2);
        return three_bytes_from_two_twelve_bits_values(i1, i2);
    }

    var repeated_bytes_from_unichars = function (u1, u2) {
        var b = int_from_u08b(u1);
        var n = int_from_u12b(u2);
        var r = [];
        for (var i = 0; i < n; i++) r.push(b);
        return r;
    }


    var two_bytes_from_unichars = function (u1, u2) {
        var b1 = int_from_u08b(u1);
        var b2 = int_from_u08b(u2);
        return [b1, b2];
    }


    var is_in_U12a = function (u) {
        var i = u.charCodeAt(0);

        for (var j = 0, start; start = [U12a_0_0_start, U12a_0_1_start, U12a_1_0_start, U12a_1_1_start][j]; j++) {
            if ((i >= start) && (i < (start + U12a_length)))
                return true;
        }
        return false;


    }

    var is_in_U8b = function (u) {
        var i = u.charCodeAt(0);
        return ((i >= U8_start) && (i < (U8_start + U8_length)));
    }

    var is_in_U12b = function (u) {
        var i = u.charCodeAt(0);
        return ((i >= U12b_start) && (i < (U12b_start + U12b_length)));
    }

    var bytes_from_u1_u2 = function (u1, u2) {
        var u1_in_U12 = is_in_U12b(u1);
        var u2_in_U12 = is_in_U12b(u2);

        var u1_in_U8 = is_in_U8b(u1);
        var u2_in_U8 = is_in_U8b(u2);

        if (u1_in_U12 && u2_in_U12)
            return three_bytes_from_unichars(u1, u2)
        else if (u1_in_U8 && u2_in_U12)
            return repeated_bytes_from_unichars(u1, u2)
        else if (u1_in_U8 && u2_in_U8)
            return two_bytes_from_unichars(u1, u2)
        else {
            console.error("--" + u1 + " " + u2 + " " + u1.charCodeAt(0).toString(16) + " " + u2.charCodeAt(0).toString(16));
            throw new Error("ValueError");
        }

    }

    var gen_encode_unichars_from_bytes = function (bytes) {
        var i = 0;

        var result = "";

        while (i < bytes.length) {
            var r = number_of_left_instances_from_index(bytes, i);

            if (r >= 3) {
                // read N bytes | N >= 3 and N < 0x1000, encode as 2 unichar
                if (r >= 0x1000) {
                    r = 0xFFF
                }

                result += two_unichr_to_repeat_byte_ntimes(bytes[i], r);

                i += r;
            } else {
                var two_ascii_chars_available = bytes.length >= i + 2 && bytes[i] < 128 && bytes[i + 1] < 128;

                if (two_ascii_chars_available) {
                    //read 2 x 7 bits, encode 1 unichar
                    result += unichr_12a_from_two_ascii(bytes[i], bytes[i + 1]);
                    i += 2;
                } else if (bytes.length >= i + 3) {
                    // read 3 bytes, encode 2 unichars

                    var s = two_twelve_bits_values_from_three_bytes(bytes[i], bytes[i + 1], bytes[i + 2]);
                    result += unichr_12_from_int(s[0])+unichr_12_from_int(s[1]);
                    i += 3;
                } else {
                    // read 1 byte, encode 1 unichar

                    result += unichr_08_from_int(bytes[i]);
                    i += 1;
                }


            }

        }
        return result;

    }

    var gen_decode_bytes_from_string = function (s) {
        var i = 0;


        // strip linebreaks
        s = s.replace(/(\r\n|\n|\r)/gm,"");

        //first pass determine size

        var bufferSize = 0;

        while (i < s.length) {
            if (s[i] == '\n') {
                i += 1;
                continue;
            }

            if (is_in_U12a(s[i])) {
                // 1 U12a -> read 2 ascii characters
                //var bytes = two_bytes_from_u12a(s[i])
                i += 1;
                bufferSize += 2;
            } else if ((i + 1) < s.length) {
                // (U12b, U12b) -> read 3 bytes
                // (U8b, U12b) -> read repetition
                // (U8b, U8b) -> read 1 byte, 1 byte
                var u1 = s[i];
                i += 1;

                while (s[i] == '\n') {
                    i += 1;
                }

                var u2 = s[i];
                i += 1;

                bytes = bytes_from_u1_u2(u1, u2)
                bufferSize += bytes.length;
            } else if (is_in_U8b(s[i])) {
                // 1 U8b -> read 1 byte
                //var b = int_from_u08b(s[i]);
                i += 1;
                //return [b];
                bufferSize += 1;
            } else {
                console.error("cannot decode " + s);
            }

        }

        var result = new Uint8Array(bufferSize);
        var resultLoc = 0;

        i = 0;
        while (i < s.length) {
            if (s[i] == '\n') {
                i += 1;
                continue;
            }

            if (is_in_U12a(s[i])) {
                // 1 U12a -> read 2 ascii characters
                var bytes = two_bytes_from_u12a(s[i])
                i += 1;
                result[resultLoc] = bytes[0];
                result[resultLoc + 1] = bytes[1];
                resultLoc += 2;

            } else if ((i + 1) < s.length) {
                // (U12b, U12b) -> read 3 bytes
                // (U8b, U12b) -> read repetition
                // (U8b, U8b) -> read 1 byte, 1 byte
                var u1 = s[i];
                i += 1;

                while (s[i] == '\n') {
                    i += 1;
                }

                var u2 = s[i];
                i += 1;

                bytes = bytes_from_u1_u2(u1, u2)
                for (var j = 0; j < bytes.length; j++) {
                    result[resultLoc + j] = bytes[j];
                }
                resultLoc += bytes.length;

            } else if (is_in_U8b(s[i])) {
                // 1 U8b -> read 1 byte
                var b = int_from_u08b(s[i]);
                i += 1;

                result[resultLoc] = b;
                resultLoc++;
            } else {
                console.error("cannot decode " + s);
            }

        }

        return result;

    }

    var decodeString = function (encoded) {

        var resultArray = gen_decode_bytes_from_string(encoded);

        var encodedString = "";
        
	for(var i=0;i<resultArray.length;i++) encodedString+=String.fromCharCode(resultArray[i]);

        return decodeURIComponent(escape(encodedString));

    }

    var encodeString = function (str) {

        var utf8 = [];
        for (var i = 0; i < str.length; i++) {
            var charcode = str.charCodeAt(i);
            if (charcode < 0x80) utf8.push(charcode);
            else if (charcode < 0x800) {
                utf8.push(0xc0 | (charcode >> 6),
                        0x80 | (charcode & 0x3f));
            }
            else if (charcode < 0xd800 || charcode >= 0xe000) {
                utf8.push(0xe0 | (charcode >> 12),
                        0x80 | ((charcode >> 6) & 0x3f),
                        0x80 | (charcode & 0x3f));
            }
            // surrogate pair
            else {
                i++;
                // UTF-16 encodes 0x10000-0x10FFFF by
                // subtracting 0x10000 and splitting the
                // 20 bits of 0x0-0xFFFFF into two halves
                charcode = 0x10000 + (((charcode & 0x3ff) << 10)
                    | (str.charCodeAt(i) & 0x3ff))
                utf8.push(0xf0 | (charcode >> 18),
                        0x80 | ((charcode >> 12) & 0x3f),
                        0x80 | ((charcode >> 6) & 0x3f),
                        0x80 | (charcode & 0x3f));
            }
        }

        return gen_encode_unichars_from_bytes(utf8);

    }

    if (typeof UNIBINARY_UNIT_TESTING == 'undefined') {

	    return {
	        encode: gen_encode_unichars_from_bytes,
	        decode: gen_decode_bytes_from_string,
	        encodeString: encodeString,
	        decodeString: decodeString
	    }

    } else {

        return {
            encode: gen_encode_unichars_from_bytes,
            decode: gen_decode_bytes_from_string,
            encodeString: encodeString,
            decodeString: decodeString,
            two_unichr_to_repeat_byte_ntimes:two_unichr_to_repeat_byte_ntimes,
            unichr_12a_from_two_ascii:unichr_12a_from_two_ascii,
            unichr_08_from_int:unichr_08_from_int,
            unichr_12_from_int:unichr_12_from_int,
            int_from_u08b:int_from_u08b,
            two_bytes_from_u12a:two_bytes_from_u12a,
            int_from_u12b:int_from_u12b,
            two_twelve_bits_values_from_three_bytes:two_twelve_bits_values_from_three_bytes,
            three_bytes_from_two_twelve_bits_values:three_bytes_from_two_twelve_bits_values,
            number_of_left_instances_from_index:number_of_left_instances_from_index,
            three_bytes_from_unichars:three_bytes_from_unichars,
            repeated_bytes_from_unichars:repeated_bytes_from_unichars,
            two_bytes_from_unichars:two_bytes_from_unichars,
            is_in_U12a:is_in_U12a,
            is_in_U8b:is_in_U8b,
            is_in_U12b:is_in_U12b,
            bytes_from_u1_u2:bytes_from_u1_u2,
            U8_start:U8_start,
            U8_length:U8_length,
            U12b_start:U12b_start,
            U12b_length:U12b_length
        }
            
    }
}));

