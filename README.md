# UniBinary

_Encodes data into printable Unicode characters._

### What is UniBinary

UniBinary is an encoding algorithm which packs arbitrary data into printable Unicode characters.

It can be used to send data through media such as Twitter which don't allow binary data but allow Unicode characters.

UniBinary is akin to Base64 but uses much fewer characters.

UniBinary comes with three parts:

- this documentation,
- a Python implementation,
- a C implementation.

### Python Implementation

Encode a binary file into a UTF-8 text file:

    $ python unibinary.py -e /bin/date > /tmp/date.txt
    $ file /tmp/date.txt 
    /tmp/date.txt: UTF-8 Unicode text, with very long lines, with no line terminators

Decode a UTF-8 text file into a binary file:
    
    $ python unibinary.py -d /tmp/date.txt > /tmp/date
    $ file /tmp/date
    /tmp/date: Mach-O 64-bit executable x86_64

It works!
    
    $ chmod +x /tmp/date
    $ /tmp/date
    Thu Jan 17 18:02:24 CET 2013

Inline string encoding:

    $ python unibinary.py -es "test"
    鬥髴

Inline string decoding:

    $ python unibinary.py -ds "嫯壭巠唀帀廀帀庀帀庀帀嚀一币帀币帀常帀済靬餯瘷駲餤悀巿巿Ѐ丅戀Ѐ丅榀帀乿巿巰叿巿崀帀丏巿巿崅帀渐帀币帀帐帀丏崀巿嵪焨最帀袁夀劃峀勍嘈凄爪与夑巰一帀ӿ丅丏巿蠀帀夀侃峀勍嘏巿巿巿帀巿崀丏巿" > micro_macho
    $ chmod +x micro_macho
    $ ./micro_macho
    Hello world

### C Implementation

Compile it with `make`:

	$ make
	$ make tests

Run the unit tests:

	$ ./tests
	...
	-- ALL TESTS ARE OK --

Run the main executable:

	$ ./unibinary
	Usage: unibinary [-ed] [-sf] [-b num] [-h]

	UniBinary encodes and decodes data into printable Unicode characters.

	  -e, --encode
	  -d, --decode
	  -s, --string    to be encoded or decoded
	  -f, --filepath  to be encoded or decoded
	  -b, --break     break encoded string into num characters lines
	  -h, --help      show this help message and exit

Encode a file, break output in lines of 16 characters:

	$ unibinary -b 16 -ef micro_macho 
	嫯壭巠唀帀廀帀庀帀庀帀嚀一币Ѐ七
	幀帀氀帀逥餬觠鯯骬蜊丏巿巰一Ѐ七
	戀Ѐ丅榀帀乿巿巰叿巿崀Ѐ七ӿ丄彀
	帀舀帀幀帀戀帀巰仿巶堌蠤Ѐ七袁夀
	劃峀勍嘈凄爪与夑巰一帀ӿ丅丏巿蠀
	帀夀侃峀勍嘏巿ӿ七帀巿崀丏巿

Encode stdin and decode the output:

	$ echo "test" | unibinary -e | unibinary -d
	test

API (`unibinary.h`)

	// encode
	int unibinary_encode(FILE *fd_in, FILE *fd_out, size_t wrap_length);
	int unibinary_encode_string(const char* src, wchar_t **dst, size_t wrap_length);

	// decode
	int unibinary_decode(FILE *src, FILE *dst);
	int unibinary_decode_string(const wchar_t *src, char **dst, long *dst_len);

Encoding and decoding are efficient and time (worst case) is linear with input size.
	
In the following example, 10 times the data take 10 times more time to encode or decode.

	# generate 50 MB of random data
 	$ dd if=/dev/urandom bs=1k count=1024*50 > /tmp/50
	$ shasum /tmp/50
	ca8554834cb036a6f7caf449f771573f82ef8b26  /tmp/50
	$ time unibinary -ef /tmp/50 > /tmp/50.txt
	user	0m8.062s
	$ time unibinary -df /tmp/50.txt > /tmp/50_decoded
	user	0m6.712s
	$ shasum /tmp/50_decoded 
	ca8554834cb036a6f7caf449f771573f82ef8b26  /tmp/50_decoded
	
	# generate 500 MB of random data
	$ dd if=/dev/urandom bs=1k count=1024*500 > /tmp/500
	$ shasum /tmp/500
	a69bacfbe3999a817cab9608d14f463fce9b2cd7  /tmp/500
	$ user	1m20.879s
	$ time unibinary -df /tmp/500.txt > /tmp/500_decoded
	user	1m7.764s
	$ shasum /tmp/500_decoded
	a69bacfbe3999a817cab9608d14f463fce9b2cd7  /tmp/500_decoded

### Encoded Text Size

UniBinary can store 3 arbitrary bytes or 4 ASCII 7-bits characters into 2 Unicode characters.

You can compare UniBinary with Base64, which stores 3 bytes into 4 ASCII characters:

            | UniBinary (Unicode) | Base64 (ASCII)
    --------+---------------------+----------------
     6 bits |                     | 1 character
    12 bits | 1 character         | 
    2 ASCII | 1 character         | 
    3 bytes | 2 characters        | 4 characters
    6 ASCII | 3 characters        | 8 characters

The worst case of encoding `N` bytes is `(N * 2 / 3 + 2)` Unicode characters.

`C` Unicode characters can store at least `(C - C % 3) * 3 / 2 + (C % 3)` bytes.

Hence, UniBinary can pack at least 209 bytes in 140 characters.

In case of a text only made out of `N` ASCII 7-bits characters, the worst case is `N / 2 + 1` Unicode characters.

Also, any repeated sequence of character will be compressed with a [run-length encoding](http://en.wikipedia.org/wiki/Run-length_encoding).

### Format Description

#### 1. Storing Data into Unicode Code Points

UniBinary packs data into three ranges of Unicode characters, named `U8`, `U12a` and `U12b`.

A character in `U8` stores a 8-bits value, a character in `U12a` or `U12b` stores a 12-bits value.

    U8   = [ \u0400, ..., \u0400 + 0x100 [

    U12b = [ \u4E00, ..., \u4E00 + 0x1000 [

    U12a_0_0 = [ \u5E00, ..., \u5E00 + 0x1000 [
    U12a_0_1 = [ \u6E00, ..., \u6E00 + 0x1000 [
    U12a_1_0 = [ \u7E00, ..., \u7E00 + 0x1000 [
    U12a_1_1 = [ \u8E00, ..., \u8E00 + 0x1000 [

`U8` is actually the "Cyrillic" block, while `U12a` and `U12b` are subsets of the "CJK Unified Ideographs" block.

`U8` and `U12b` store arbitrary 8 and 12 bits sequences, while the `U12a` blocks are used to store ASCII 7-bits characters.

The offset in the range represent the bits to be encoded.

    0xAB  (8 bits)  gets encoded as \u0400 + 0xAB  = \u04AB = ҫ
    0xABC (12 bits) gets encoded as \u4E00 + 0xABC = \u58BC = 뱘

#### 2. Mapping Arbitrary Bytes into Unicode

UniBinary reads three bytes to yield two Unicode characters in the `U12b` range .

Here is how UniBinary encode the 24 bits value `0xABCDEF` into two Unicode characters, and how Base64 does it by comparision:

            UniBinary                |                 Base64
                                     |
    A   B   |C   D   |E   F          |        A   B   |C   D   |E   F   
    10101011 11001101 11101111       |        10101011 11001101 11101111
    [-----------][-----------]       |        [----][-----][-----][----]
         ABC          DEF            |        101010 111100 110111 101111
    \u4E00+0xABC \u4E00+0xDEF        |          42     60     55     47
          墼           寯             |          q      8      3      v

If less than three bytes are available, UniBinary reads bytes one by one to yield Unicode characters in `U8`.

    A   B   
    10101011
    [------]
       AB
    \u0400+0xAB
       ҫ

#### 3. Mapping ASCII 7-bits into one Unicode character

When UniBinary meets 2 ASCII 7-bits characters `a1` and `a2`, it encodes them into one single Unicode character. This character is chosen out of four possible ranges, depending on the value of the ASCII characters:

    U12a_0_0    [ \u5E00, ..., \u5E00 + 0x1000 [     for a1 <  64 and a2 <  64
    U12a_0_1    [ \u6E00, ..., \u6E00 + 0x1000 [     for a1 <  64 and a2 >= 64
    U12a_1_0    [ \u7E00, ..., \u7E00 + 0x1000 [     for a1 >= 64 and a2 <  64
    U12a_1_1    [ \u8E00, ..., \u8E00 + 0x1000 [     for a1 >= 64 and a2 >= 64

So, we can pack 2 * 6 bits in a `U12a` Unicode character. We use four different ranges to replace the 7th (MSB) missing bit. We use `U12a_1_0` and `U12a_1_1` to add 64 to `a1`, and `U12a_0_1` and `U12a_1_1` to add 64 to `a2`. As a result, we can store any tuple of 2 ASCII 7-bits characters in a single Unicode character.

#### 4. Run Length Encoding 

UniBinary also takes advantage of repetitions to spare bytes. A byte `B` repeated more that 3 times gets encoded as `(u8, u12)` where `u8` stores `B` and `u12` stores the number of times that `B` is repeated in the `U12b` range.

#### 5. Format Summary
    
    - u8   u12b ->  byte B (u8) repeated N times (u12) | N in [3, 0xFFF]
    - u12a      ->  12 bits (2 ASCII characters)
    - u12b u12b ->  24 bits (3 bytes)
    - u8        ->  8 bits (1 byte)

UniBinary encoded data can be described with the following regular expression:

    ( u12a | (u12 u12) | (u8 u12) )* u8 {0,2}
    
Note that new lines (`\n`) can appear anywhere in the encoded text. The decoding algorithm does simply ignore them.

#### 6. Examples

    0x12 0x34           -> encode 0x12 into U8, encode 0x34 into U8
    0xAB 0xCD 0xEF      -> encode 0xABC into U12b, encode 0xDEF into U12b
    0xFF 0xFF 0xFF 0xFF -> encode 0xFF into U8, encode 0x4 into U12b

    AB CD EF FF FF FF FF 00 -> U12(0xABC), U12(0xDEF), U8(4), U12(0xFF), U8(00) -> "墼寯巿巿Ѐ"

    13808 bytes /usr/bin/true -> 3253 Unicode characters, 9721 bytes UTF-8 file

### Encoding Algorithm

First look for repetitions (no more than `0xFFF` at a time). If no repeat, then try to consume two ASCII chars. If it's not possible, look for three bytes. If less than three bytes are available, encode one byte at a time.

    1. byte B repeated N times | N >= 3   ->    U8(B), U12(N)
    2. ASCII characters A1, A2            ->    U12a(A1, A2)
    3. bytes B1, B2, B3                   ->    U12b(B1 << 4 + B2 >> 4), U12b(((B2 & 0xF) << 8) + B3)
    4. byte B                             ->    U8(B)

### Decoding Algorithm

For each unicode character, use the range to know how to unmarshall data. Extract two ASCII characters out of `U12a`, or `N` times `B` out of `(U8, U12b)`, or three bytes out of `(U12b, U12b)`, or one bytes out of `U8`.

See to the source code for implementation details.
