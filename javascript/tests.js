UNIBINARY_UNIT_TESTING=1;

if (typeof require != 'undefined') {
if (typeof QUnit == 'undefined') QUnit = require('qunit-cli');
if (typeof unibinary =='undefined') unibinary = require('./unibinary');
}

QUnit.module("unibinary");

QUnit.test("test_unichr_12_encoding_decoding", function (assert) {

    var testArray = [0x0, 0x1, 0xAB, 0x123, 0xABC, 0xF, 0xFF, 0xFFF];

    for (var j = 0; j < testArray.length; j++) {
        i = testArray[j];

        var u = unibinary.unichr_12_from_int(i);
        assert.notEqual(u, i);

        var i2 = unibinary.int_from_u12b(u);
        assert.equal(i, i2);
    }

});

QUnit.test("test_3_to_2_bytes", function (assert) {

    var ab = unibinary.two_twelve_bits_values_from_three_bytes(0x12, 0x34, 0x56);

    assert.equal(ab[0], 0x123, "0x" + ab[0].toString(16));
    assert.equal(ab[1], 0x456, "0x" + ab[1].toString(16));

});

QUnit.test("test_2_to_3_bytes", function (assert) {

    var abc = unibinary.three_bytes_from_two_twelve_bits_values(0x123, 0x456);

    assert.equal(abc[0], 0x12, "0x" + abc[0].toString(16));
    assert.equal(abc[1], 0x34, "0x" + abc[1].toString(16));
    assert.equal(abc[2], 0x56, "0x" + abc[2].toString(16));

});

QUnit.test("test_encode_3_bytes", function (assert) {

    var bytes = [0xab, 0xcd, 0xef];
    var gen = unibinary.encode(bytes);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 2);

    assert.equal(gen.charCodeAt(0), unibinary.U12b_start + 0xABC);
    assert.equal(gen.charCodeAt(1), unibinary.U12b_start + 0xDEF);

});

QUnit.test("test_encode_bytes", function (assert) {
    var bytes = [0xab, 0xcd, 0xef, 0xff];

    var gen = unibinary.encode(bytes);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 3);

    assert.equal(gen.charCodeAt(0), unibinary.U12b_start + 0xABC);
    assert.equal(gen.charCodeAt(1), unibinary.U12b_start + 0xDEF);
    assert.equal(gen.charCodeAt(2), unibinary.U8_start + 0xFF);

});

QUnit.test("test_decode_unichars", function (assert) {

    var u1 = String.fromCharCode(unibinary.U12b_start + 0xABC);
    var u2 = String.fromCharCode(unibinary.U12b_start + 0xDEF);

    var s = u1 + u2;

    var gen = unibinary.decode(s);

    assert.equal(gen.length, 3);

    assert.equal(gen[0], 0xAB);
    assert.equal(gen[1], 0xCD);
    assert.equal(gen[2], 0xEF);

});

QUnit.test("test_is_in_U8b", function (assert) {
    assert.ok(!unibinary.is_in_U8b(String.fromCharCode(0x03FF)));
    assert.ok(unibinary.is_in_U8b(String.fromCharCode(0x400)));
    assert.ok(unibinary.is_in_U8b(String.fromCharCode(0x4FF)));
    assert.ok(!unibinary.is_in_U8b(String.fromCharCode(0x500)));
});

QUnit.test("test_unichr_12a_from_two_ascii", function (assert) {
    var u = unibinary.unichr_12a_from_two_ascii('Z'.charCodeAt(0), 'E'.charCodeAt(0));
    assert.equal(u, String.fromCharCode(0x9485));

    var u = unibinary.unichr_12a_from_two_ascii('z'.charCodeAt(0), ','.charCodeAt(0));
    assert.equal(u, String.fromCharCode(0x8CAC));

});

QUnit.test("test_ascii_characters_encoding", function (assert) {
    var s = "abc";

    var gen = unibinary.encodeString(s);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 2);

    assert.equal(gen.charCodeAt(0), 0x9662);
    assert.equal(gen.charCodeAt(1), 0x0463);

});

QUnit.test("test_ascii_characters_encoding_2", function (assert) {
    var s = "ZE";

    var gen = unibinary.encodeString(s);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 1);

    assert.equal(gen[0], unibinary.unichr_12a_from_two_ascii('Z'.charCodeAt(0), 'E'.charCodeAt(0)));

});

QUnit.test("test_two_unichr_to_repeat_byte_ntimes_aaa", function (assert) {

    var gen = unibinary.two_unichr_to_repeat_byte_ntimes('a'.charCodeAt(0), 10);
    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 2);

    assert.equal(gen.charCodeAt(0), 0x0461);
    assert.equal(gen.charCodeAt(1), 0x4E0A);

});

QUnit.test("test_two_unichr_to_repeat_byte_ntimes_xxx", function (assert) {

    var gen = unibinary.two_unichr_to_repeat_byte_ntimes('x'.charCodeAt(0), 3);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 2);

    assert.equal(gen.charCodeAt(0), 0x0478);
    assert.equal(gen.charCodeAt(1), 0x4E03);

});

QUnit.test("test_repeat", function (assert) {

    var s = "xxx";

    var gen = unibinary.encodeString(s);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 2);

    assert.equal(gen.charCodeAt(0), 0x0478);
    assert.equal(gen.charCodeAt(1), 0x4E03);

});

QUnit.test("test_ascii_characters_decoding", function (assert) {
    var s = String.fromCharCode(0x9662) + String.fromCharCode(0x0463);

    var s2 = unibinary.decode(s);

    assert.equal(s2[0], 'a'.charCodeAt(0));
    assert.equal(s2[1], 'b'.charCodeAt(0));
    assert.equal(s2[2], 'c'.charCodeAt(0));


});

QUnit.test("test_ascii_characters_decoding_2", function (assert) {
    var s = String.fromCharCode(0x9485);

    var s2 = unibinary.decode(s);

    assert.equal(s2[0], 'Z'.charCodeAt(0));
    assert.equal(s2[1], 'E'.charCodeAt(0));


});

QUnit.test("test_five_bytes_encoding", function (assert) {
    var bytes = [0xab, 0xcd, 0xef, 0xab, 0xcd];

    var gen = unibinary.encode(bytes);

    assert.equal(gen.length, 4);

    assert.equal(gen.charCodeAt(0), unibinary.U12b_start + 0xABC);
    assert.equal(gen.charCodeAt(1), unibinary.U12b_start + 0xDEF);
    assert.equal(gen.charCodeAt(2), unibinary.U8_start + 0xAB);
    assert.equal(gen.charCodeAt(3), unibinary.U8_start + 0xCD);

});

QUnit.test("test_ascii_and_bytes_encoding", function (assert) {
    var bytes = [0xab, 0xcd, 0xef];
    bytes = bytes.concat([0x61, 0x62, 0x63, 0x64, 0x65]); //abcde

    var gen = unibinary.encode(bytes);

    assert.equal(gen.length, 5);

    assert.equal(gen.charCodeAt(0), unibinary.U12b_start + 0xABC);
    assert.equal(gen.charCodeAt(1), unibinary.U12b_start + 0xDEF);
    assert.equal(gen[2], unibinary.unichr_12a_from_two_ascii('a'.charCodeAt(0), 'b'.charCodeAt(0)));
    assert.equal(gen[3], unibinary.unichr_12a_from_two_ascii('c'.charCodeAt(0), 'd'.charCodeAt(0)));
    assert.equal(gen[4], unibinary.unichr_08_from_int('e'.charCodeAt(0)));


});

QUnit.test("test_ascii_and_bytes_decoding", function (assert) {
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


});

QUnit.test("test_repeats", function (assert) {
    var l = [1, 1, 1, 2, 1];

    var n = unibinary.number_of_left_instances_from_index(l, 0);

    assert.equal(n, 3);

});

QUnit.test("test_empty_string", function (assert) {
    var bytes = "";

    var gen = unibinary.encodeString(bytes);

    assert.equal(gen, "");

});

QUnit.test("test_one_char", function (assert) {
    var bytes = "a";

    var gen = unibinary.encodeString(bytes);

    assert.ok(typeof gen == "string");
    assert.equal(gen.length, 1);


    assert.equal(gen.charCodeAt(0), 0x0461);

});

QUnit.test("test_repeats_2", function (assert) {

    var bytes = [0xAB, 0xCD, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00];

    var gen = unibinary.encode(bytes);

    assert.equal(gen.length, 5);

    assert.equal(gen.charCodeAt(0), 0x58BC);
    assert.equal(gen.charCodeAt(1), 0x5BEF);
    assert.equal(gen.charCodeAt(2), 0x04FF);
    assert.equal(gen.charCodeAt(3), 0x4E04);
    assert.equal(gen.charCodeAt(4), 0x0400);

});

QUnit.test("test_encode_macho_header", function (assert) {

    var bytes = [0xCF, 0xFA, 0xED, 0xFE, 0x07, 0x00, 0x00, 0x01];

    var gen = unibinary.encode(bytes);

    assert.equal(gen.length, 5);

    assert.equal(gen.charCodeAt(0), 0x5AFF);
    assert.equal(gen.charCodeAt(1), 0x58ED);
    assert.equal(gen.charCodeAt(2), 0x5DE0);
    assert.equal(gen.charCodeAt(3), 0x5500);
    assert.equal(gen.charCodeAt(4), 0x5E01);

});

QUnit.test("test_big_repeats_2000_minus_2", function (assert) {

    var bytes = [];
    for(var i=0;i<0x2000-2;i++) bytes.push(0xAA);

    var gen = unibinary.encode(bytes);

    assert.equal(gen.length, 4);

    assert.equal(gen.charCodeAt(0), 0x04AA);
    assert.equal(gen.charCodeAt(1), 0x5DFF);
    assert.equal(gen.charCodeAt(2), 0x04AA);
    assert.equal(gen.charCodeAt(3), 0x5DFF);

});

QUnit.test("test_big_repeats_2000", function (assert) {

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

});

QUnit.test("test_ascii_text_encoding_decoding", function (assert) {

    var s = "if I'd listened everything that they said to me, took the time to bleed from all the tiny little arrows shot my way, I wouldn't be here! the ones who don't do anything are always the ones who try to put you down. I'm talking to you: hero time starts right now! time to shine!";

    var encodeGen = unibinary.encodeString(s);

    var decodeGen = unibinary.decodeString(encodeGen);

    assert.equal(s,decodeGen);

});

QUnit.test("test_ascii_text_encoding_decoding_2", function (assert) {

    var s = "";
    for (var i=32; i<=128;i++) s+=String.fromCharCode(i);

    var encodeGen = unibinary.encodeString(s);

    var decodeGen = unibinary.decodeString(encodeGen);

    assert.equal(s,decodeGen);

});

