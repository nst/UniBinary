#!/usr/bin/env node

/**
 * node.js command line tool for unibinary.js
 *
 * Author:
 * Toolsley, 2014-12-03
 *
 * License: BSD
 *
 */

var fs = require('fs');
var constants = require('constants');
var unibinary = require('./unibinary');

function toArrayBuffer(buffer) {
    var ab = new ArrayBuffer(buffer.length);
    var view = new Uint8Array(ab);
    for (var i = 0; i < buffer.length; ++i) {
        view[i] = buffer[i];
    }
    return ab;
}

function toBuffer(view) {
    var buffer = new Buffer(view.length);
    for (var i = 0; i < buffer.length; ++i) {
        buffer[i] = view[i];
    }
    return buffer;
}

switch (process.argv[2]) {
    case "-es":
    case "--encode_string":
        console.log(unibinary.encodeString(process.argv[3]));
        break;
    case "-ds":
    case "--decode_string":
        console.log(process.argv[3]);
        console.log(unibinary.decodeString(process.argv[3]));
        break;
    case "-e":
    case "--encode":
        fs.readFile(process.argv[3],function (err, data) {
            if (err) throw err;
            var dataArray = new Uint8Array(toArrayBuffer(data));
            process.stdout.write(unibinary.encode(dataArray));
        });
        break;
    case "-d":
    case "--decode":
        fs.readFile(process.argv[3],"utf-8",function (err, data) {
            if (err) throw err;
            var dataBuf = toBuffer(unibinary.decode(data));
            process.stdout.write(dataBuf);
        });
        break;
    default:

        console.log("usage: unibinary_tool.js [-h] [-e ENCODE] [-d DECODE] [-es ENCODE_STRING]\n\
                         [-ds DECODE_STRING]\n\
        \n\
        UniBinary encodes and decodes data into printable Unicode characters.\n\
        \n\
        optional arguments:\n\
        -h, --help            show this help message and exit\n\
        -e ENCODE, --encode ENCODE\n\
            file to encode\n\
        -d DECODE, --decode DECODE\n\
            file to decode\n\
        -es ENCODE_STRING, --encode_string ENCODE_STRING\n\
            utf-8 string to encode\n\
        -ds DECODE_STRING, --decode_string DECODE_STRING\n\
            utf-8 string to decode\n");
}