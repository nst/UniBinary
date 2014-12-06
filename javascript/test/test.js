/**
 * Node, Rhino, JSC, and Browser compatible wru test runner.
 */

	UNIBINARY_UNIT_TESTING = true;	
	var global;
	var Uint8Array;
	var unibinary;
	var wru;
	var console;

        // node, rhino, or web
        try {
            // node and phantom js

            wru = require("./wru.console.js");
	    unibinary = require("../unibinary");
	    
            go(wru);
        } catch(e) {
            // rhino
            try {	

		global = undefined; // this is a hack to work around a bug in wru & new Rhino versions
		load("../unibinary.js");
		load("./wru.console.js");

		if (typeof Uint8Array == "undefined") {
	                load("typedarray.js");
        		Uint8Array = exports.Uint8Array;
		}
				
		if (typeof console == "undefined") {
			console = {
				error: function(msg) {print(msg)},
				log: function(msg) {print(msg)}			
			}
		}

                go(wru);
            } catch(e) {
                try {
                    // jsc test/test.js
                    load("../wru.console.js");
                    jsc = true;
                    load("../unibinary.js");

                    go(wru);
                } catch(e) {
                    // html (assuming test.html is used in same folders structure)
                    (function(xhr){
                        try {
                        xhr.open("get", "wru.min.js", true);
                        xhr.onreadystatechange = function () {
                          if (xhr.readyState == 4) {
                            try {
                              Function(xhr.responseText.replace(/var wru=/,"this.wru=")).call(window);
                            } catch(e) {
                              alert(e);
                            }
                            go(window.wru);
                          }
                        };
                        xhr.send(null);
                        } catch(e) {
                          alert(e.message || e);
                        }
                    }(new XMLHttpRequest));
                }
            }
        }

  function go(wru) {
	var assert = {
		equal : function(a,b,m) {
			wru.assert(a==b);
		},
		notEqual : function(a,b,m) {
			wru.assert(a!=b);
		},

		ok : function(a,m) {
			wru.assert(a);
		}
	}

        wru.test([{
            name: "test_unichr_12_encoding_decoding",
            test: function () {

	    var testArray = [0x0, 0x1, 0xAB, 0x123, 0xABC, 0xF, 0xFF, 0xFFF];

	    for (var j = 0; j < testArray.length; j++) {
	        i = testArray[j];
	
	        var u = unibinary.unichr_12_from_int(i);
	        assert.notEqual(u,i);
	
	        var i2 = unibinary.int_from_u12b(u);
	        assert.equal(i,i2);
	    }

	}},{

name:"test_3_to_2_bytes", test: function () {

    var ab = unibinary.two_twelve_bits_values_from_three_bytes(0x12, 0x34, 0x56);

    assert.equal(ab[0] , 0x123, "0x" + ab[0].toString(16));
    assert.equal(ab[1] , 0x456, "0x" + ab[1].toString(16));

}},{

name:"test_2_to_3_bytes", test: function() {

    var abc = unibinary.three_bytes_from_two_twelve_bits_values(0x123, 0x456);

    assert.equal(abc[0], 0x12, "0x" + abc[0].toString(16));
    assert.equal(abc[1], 0x34, "0x" + abc[1].toString(16));
    assert.equal(abc[2], 0x56, "0x" + abc[2].toString(16));

}},{

name:"test_encode_3_bytes", test: function() {

    var bytes = [0xab, 0xcd, 0xef];
    var gen = unibinary.encode(bytes);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 2);

    assert.equal(gen.charCodeAt(0), unibinary.U12b_start + 0xABC);
    assert.equal(gen.charCodeAt(1), unibinary.U12b_start + 0xDEF);

}},{

name:"test_encode_bytes", test: function() {
    var bytes = [0xab, 0xcd, 0xef, 0xff];

    var gen = unibinary.encode(bytes);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 3);

    assert.equal(gen.charCodeAt(0), unibinary.U12b_start + 0xABC);
    assert.equal(gen.charCodeAt(1), unibinary.U12b_start + 0xDEF);
    assert.equal(gen.charCodeAt(2), unibinary.U8_start + 0xFF);

}},{

name:"test_decode_unichars", test: function() {

    var u1 = String.fromCharCode(unibinary.U12b_start + 0xABC);
    var u2 = String.fromCharCode(unibinary.U12b_start + 0xDEF);

    var s = u1 + u2;

    var gen = unibinary.decode(s);

    assert.equal(gen.length, 3);

    assert.equal(gen[0], 0xAB);
    assert.equal(gen[1], 0xCD);
    assert.equal(gen[2], 0xEF);

}},{

name:"test_is_in_U8b", test: function() {
    assert.ok(!unibinary.is_in_U8b(String.fromCharCode(0x03FF)));
    assert.ok(unibinary.is_in_U8b(String.fromCharCode(0x400)));
    assert.ok(unibinary.is_in_U8b(String.fromCharCode(0x4FF)));
    assert.ok(!unibinary.is_in_U8b(String.fromCharCode(0x500)));
}},{

name:"test_unichr_12a_from_two_ascii", test: function() {
    var u = unibinary.unichr_12a_from_two_ascii('Z'.charCodeAt(0), 'E'.charCodeAt(0));
    assert.equal(u, String.fromCharCode(0x9485));

    var u = unibinary.unichr_12a_from_two_ascii('z'.charCodeAt(0), ','.charCodeAt(0));
    assert.equal(u, String.fromCharCode(0x8CAC));

}},{

name:"test_ascii_characters_encoding", test: function() {
    var s = "abc";

    var gen = unibinary.encodeString(s);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 2);

    assert.equal(gen.charCodeAt(0), 0x9662);
    assert.equal(gen.charCodeAt(1), 0x0463);

}},{

name:"test_ascii_characters_encoding_2", test: function() {
    var s = "ZE";

    var gen = unibinary.encodeString(s);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 1);

    assert.equal(gen[0], unibinary.unichr_12a_from_two_ascii('Z'.charCodeAt(0), 'E'.charCodeAt(0)));

}},{

name:"test_two_unichr_to_repeat_byte_ntimes_aaa", test: function() {

    var gen = unibinary.two_unichr_to_repeat_byte_ntimes('a'.charCodeAt(0), 10);
    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 2);

    assert.equal(gen.charCodeAt(0), 0x0461);
    assert.equal(gen.charCodeAt(1), 0x4E0A);

}},{

name:"test_two_unichr_to_repeat_byte_ntimes_xxx", test: function() {

    var gen = unibinary.two_unichr_to_repeat_byte_ntimes('x'.charCodeAt(0), 3);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 2);

    assert.equal(gen.charCodeAt(0), 0x0478);
    assert.equal(gen.charCodeAt(1), 0x4E03);

}},{

name:"test_repeat", test: function() {

    var s = "xxx";

    var gen = unibinary.encodeString(s);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 2);

    assert.equal(gen.charCodeAt(0), 0x0478);
    assert.equal(gen.charCodeAt(1), 0x4E03);

}},{

name:"test_ascii_characters_decoding", test: function() {
    var s = String.fromCharCode(0x9662) + String.fromCharCode(0x0463);

    var s2 = unibinary.decode(s);

    assert.equal(s2[0], 'a'.charCodeAt(0));
    assert.equal(s2[1], 'b'.charCodeAt(0));
    assert.equal(s2[2], 'c'.charCodeAt(0));


}},{

name:"test_ascii_characters_decoding_2", test: function() {
    var s = String.fromCharCode(0x9485);

    var s2 = unibinary.decode(s);

    assert.equal(s2[0], 'Z'.charCodeAt(0));
    assert.equal(s2[1], 'E'.charCodeAt(0));


}},{

name:"test_five_bytes_encoding", test: function() {
    var bytes = [0xab, 0xcd, 0xef, 0xab, 0xcd];

    var gen = unibinary.encode(bytes);

    assert.equal(gen.length, 4);

    assert.equal(gen.charCodeAt(0), unibinary.U12b_start + 0xABC);
    assert.equal(gen.charCodeAt(1), unibinary.U12b_start + 0xDEF);
    assert.equal(gen.charCodeAt(2), unibinary.U8_start + 0xAB);
    assert.equal(gen.charCodeAt(3), unibinary.U8_start + 0xCD);

}},{

name:"test_ascii_and_bytes_encoding", test: function() {
    var bytes = [0xab, 0xcd, 0xef];
    bytes = bytes.concat([0x61, 0x62, 0x63, 0x64, 0x65]); //abcde

    var gen = unibinary.encode(bytes);

    assert.equal(gen.length, 5);

    assert.equal(gen.charCodeAt(0), unibinary.U12b_start + 0xABC);
    assert.equal(gen.charCodeAt(1), unibinary.U12b_start + 0xDEF);
    assert.equal(gen[2], unibinary.unichr_12a_from_two_ascii('a'.charCodeAt(0), 'b'.charCodeAt(0)));
    assert.equal(gen[3], unibinary.unichr_12a_from_two_ascii('c'.charCodeAt(0), 'd'.charCodeAt(0)));
    assert.equal(gen[4], unibinary.unichr_08_from_int('e'.charCodeAt(0)));


}},{

name:"test_ascii_and_bytes_decoding", test: function() {
    var s = String.fromCharCode(unibinary.U12b_start + 0xABC);
    s += String.fromCharCode(unibinary.U12b_start + 0xDEF);
    s += unibinary.unichr_12a_from_two_ascii('a'.charCodeAt(0), 'b'.charCodeAt(0));
    s += unibinary.unichr_12a_from_two_ascii('c'.charCodeAt(0), 'd'.charCodeAt(0));
    s += unibinary.unichr_08_from_int('e'.charCodeAt(0));

    var gen = unibinary.decode(s);

    assert.equal(gen.length, 8);

    assert.equal(gen[0], 0xAB);
    assert.equal(gen[1], 0xCD);
    assert.equal(gen[2], 0xEF);
    assert.equal(gen[3], 0x61);
    assert.equal(gen[4], 0x62);
    assert.equal(gen[5], 0x63);
    assert.equal(gen[6], 0x64);
    assert.equal(gen[7], 0x65);


}},{

name:"test_repeats", test: function() {
    var l = [1, 1, 1, 2, 1];

    var n = unibinary.number_of_left_instances_from_index(l, 0);

    assert.equal(n, 3);

}},{

name:"test_empty_string", test: function() {
    var bytes = "";

    var gen = unibinary.encodeString(bytes);

    assert.equal(gen, "");

}},{

name:"test_one_char", test: function() {
    var bytes = "a";

    var gen = unibinary.encodeString(bytes);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 1);


    assert.equal(gen.charCodeAt(0), 0x0461);

}},{

name:"test_repeats_2", test: function() {

    var bytes = [0xAB, 0xCD, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00];

    var gen = unibinary.encode(bytes);

    assert.equal(gen.length, 5);

    assert.equal(gen.charCodeAt(0), 0x58BC);
    assert.equal(gen.charCodeAt(1), 0x5BEF);
    assert.equal(gen.charCodeAt(2), 0x04FF);
    assert.equal(gen.charCodeAt(3), 0x4E04);
    assert.equal(gen.charCodeAt(4), 0x0400);

}},{

name:"test_encode_macho_header", test: function() {

    var bytes = [0xCF, 0xFA, 0xED, 0xFE, 0x07, 0x00, 0x00, 0x01];

    var gen = unibinary.encode(bytes);

    assert.equal(gen.length, 5);

    assert.equal(gen.charCodeAt(0), 0x5AFF);
    assert.equal(gen.charCodeAt(1), 0x58ED);
    assert.equal(gen.charCodeAt(2), 0x5DE0);
    assert.equal(gen.charCodeAt(3), 0x5500);
    assert.equal(gen.charCodeAt(4), 0x5E01);

}},{

name:"test_big_repeats_2000_minus_2", test: function() {

    var bytes = [];
    for(var i=0;i<0x2000-2;i++) bytes.push(0xAA);

    var gen = unibinary.encode(bytes);

    assert.equal(gen.length, 4);

    assert.equal(gen.charCodeAt(0), 0x04AA);
    assert.equal(gen.charCodeAt(1), 0x5DFF);
    assert.equal(gen.charCodeAt(2), 0x04AA);
    assert.equal(gen.charCodeAt(3), 0x5DFF);

}},{

name:"test_big_repeats_2000", test: function() {

    var bytes = [];
    for(var i=0;i<0x2000;i++) bytes.push(0xAA);

    var gen = unibinary.encode(bytes);
    assert.equal(gen.length, 6);

    assert.equal(gen.charCodeAt(0), 0x04AA);
    assert.equal(gen.charCodeAt(1), 0x5DFF);
    assert.equal(gen.charCodeAt(2), 0x04AA);
    assert.equal(gen.charCodeAt(3), 0x5DFF);
    assert.equal(gen.charCodeAt(4), 0x04AA);
    assert.equal(gen.charCodeAt(5), 0x04AA);

}},{

name:"test_ascii_text_encoding_decoding", test: function() {

    var s = "if I'd listened everything that they said to me, took the time to bleed from all the tiny little arrows shot my way, I wouldn't be here! the ones who don't do anything are always the ones who try to put you down. I'm talking to you: hero time starts right now! time to shine!";

    var encodeGen = unibinary.encodeString(s);

    var decodeGen = unibinary.decodeString(encodeGen);

    assert.equal(s,decodeGen);

}},{

name:"test_ascii_text_encoding_decoding_2", test: function() {

    var s = "";
    for (var i=32; i<=128;i++) s+=String.fromCharCode(i);

    var encodeGen = unibinary.encodeString(s);

    var decodeGen = unibinary.decodeString(encodeGen);

    assert.equal(s,decodeGen);

}}]);}
