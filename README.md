# UniBinary

_A data-to-unicode encoding format. Think "Base64 for Unicode" with RLE compression._

### What is UniBinary

UniBinary is an encoding format that maps data to Unicode characters.

UniBinary stores 3 arbitrary bytes or 4 ASCII 7-bits characters into 2 Unicode characters.

You can compare UniBinary with Base64, which stores 3 bytes into 4 ASCII characters:

            | UniBinary (Unicode) | Base64 (ASCII)
    --------+---------------------+----------------
     6 bits |                     | 1 character
    12 bits | 1 character         | 
    2 ASCII | 1 character         | 
    3 bytes | 2 characters        | 4 characters
    6 ASCII | 3 characters        | 8 characters

As Base64, UniBinary uses only displayable characters that you can copy / paste.

### Usage

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

### Format Description

#### 1. Storing Arbitrary Bytes into Unicode

UniBinary stores arbitrary data into two ranges of Unicode characters, `U8` and `U12b`.

A character in `U8` stores a 8-bits value, a character in `U12b` stores a 12-bits value.

    u8_offset = \u0400
    u8_length = 0x100
    
    u12b_offset = \u4E00
    u12b_length = 0x1000    

    U8   = [   u8_offset, ...,   u8_offset + u8_length   [
    U12b = [ u12b_offset, ..., u12b_offset + u12b_length [

`U8` is actually the "Cyrillic" block, while `U12b` is a subset of the "CJK Unified Ideographs" block.

Characters in `U8` can encode any 8-bits value by adding that value to the range starting offset. It goes the same with `U12b`.

    0xAB  (8 bits)  gets encoded as \u0400 + 0xAB  = \u04AB = ҫ
    0xABC (12 bits) gets encoded as \u4E00 + 0xABC = \u58BC = 뱘

#### 2. Mapping Bytes into Unicode

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

#### 2. Mapping ASCII 7-bits into one Unicode character

When UniBinary meets 2 ASCII 7-bits characters `a1` and `a2`, it encodes them into one single Unicode character. This character is chosen out of four possible ranges, depending on the value of the ASCII characters:

    U12a_0_0    [ 0x5E00, ..., 0x5E00 + 0xFFF [     for a1 <  64 and a2 <  64
    U12a_0_1    [ 0x6E00, ..., 0x6E00 + 0xFFF [     for a1 <  64 and a2 >= 64
    U12a_1_0    [ 0x7E00, ..., 0x7E00 + 0xFFF [     for a1 >= 64 and a2 <  64
    U12a_1_1    [ 0x8E00, ..., 0x8E00 + 0xFFF [     for a1 >= 64 and a2 >= 64

So, we can pack 2 * 6 bits in a `U12a` Unicode character. We use four different ranges to replace the 7th (MSB) missing bit. We use `U12a_1_0` and `U12a_1_0` to add 64 to `a1`, and `U12a_0_1` and `U12a_1_1` to add 64 to `a2`. As a result, we can store any tuple of 2 ASCII 7-bits characters in a single Unicode character.

#### 3. Run Length Encoding 

UniBinary also takes advantage of repetitions to spare bytes. A byte `B` repeated more that 3 times gets encoded as `(u8, u12)` where `u8` stores `B` and `u12` stores the number of times that `B` is repeated un the `U12b` range.

#### 4. Format Summary
    
    - u8   u12b ->  byte B (u8) repeated N times (u12) | N in [3, 0xFFF]
    - u12a      ->  12 bits (2 ASCII characters)
    - u12b u12b ->  24 bits (3 bytes)
    - u8        ->  8 bits (1 byte)

UniBinary encoded data can be described with the following regular expression:

    ( u12a | (u12 u12) | (u8 u12) )* u8 {0,2}

#### 4. Examples

    0x12 0x34           -> encode 0x12 into U8, encode 0x34 into U8
    0xAB 0xCD 0xEF      -> encode 0xABC into U12b, encode 0xDEF into U12b
    0xFF 0xFF 0xFF 0xFF -> encode 0xFF into U8, encode 0x4 into U12b

    AB CD EF FF FF FF FF 00 -> U12(0xABC), U12(0xDEF), U8(4), U12(0xFF), U8(00) -> "墼寯巿巿Ѐ"

    13808 bytes /usr/bin/true -> 3253 Unicode characters, 9667 bytes UTF-16 files including BOM

#### 5. Encoded Text Size

The worst case of encoding `N` bytes is `(N * 2 / 3 + 2)` Unicode characters.

`C` Unicode characters can store at least `(C - C % 3) * 3 / 2 + (C % 3)` bytes.

Hence, UniBinary can pack at least 209 bytes in the 140 characters of a Twitter message.

In case of a text only made out of `N` ASCII 7-bits characters, the worst case is `N / 2 + 1` Unicode characters.

### Encoding Algorithm

First look for repetitions (no more that 0xFFF at a time). If no repeat, then try to consume two ASCII chars. If it's not possible, look for three bytes. If less than three bytes are available, encode one byte at a time.

    1. byte B repeated N times | N > 3    ->    U8(B), U12(N)
    2. ASCII characters A1, A2            ->    U12a(A1, A2)
    3. bytes B1, B2, B3                   ->    U12b(B1 << 4 + B2 >> 4), U12b(((B2 & 0xF) << 8) + B3)
    4. byte B                             ->    U8(B)

### Decoding Algorithm

Reading all the unichars, use the Unicode range to know how to unmarshall data. Extract two ASCII characters out of `U12a`, or `N` times `B` out of `(U8, U12b)`, or three bytes out of `(U12b, U12b)`, or one bytes out of `U8`.

See to the source code for implementation details.
