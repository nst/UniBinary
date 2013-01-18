# UniBinary

_A binary-to-text encoding format. Encodes 3 bytes in 2 Unicode character. Think "Base64 for Unicode"._

### What is UniBinary

UniBinary is an encoding format to encode binary data into Unicode text.

UniBinary stores 3 bytes in 2 Unicode characters, where Base64 uses 4 ASCII characters.

UniBinary uses displayable Unicode characters that you can copy / paste.

            | UniBinary (Unicode) | Base64 (ASCII)
    --------+---------------------+----------------
     6 bits |                     | 1 character
    12 bits | 1 character         | 
    3 bytes | 2 character         | 4 characters

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

Inline string decoding:

    $ python unibinary.py -s "嫯壭巠唀一七一一丠一一倀一予一一丐一一伀一丸一一劆卬哆崠啶嵲哆刊丏巿巰一一一伀一Ѐ丄僠一一嗿巿崅巿巿Ѐ丄ӿ丄乐一丅一一丁一一伀一丏崀巿嵪仆嘤一一咠侰么凬乌宀嘼刐咠仫伟崀一一ӿ丅丏巿咀一下丁嘾娄嫘仿ӿ丄一仿巰一ӿӿ" > micro_macho
    $ chmod +x micro_macho
    $ ./micro_macho
    Hello world

### Format Description

#### 1. Storing Data into Unicode

UniBinary uses two ranges of Unicode characters, `U8` and `U12`.

A character in `U8` stores a 8-bits value, a character in `U12` stores a 12-bits value.

    u8_offset = \u0400
    u8_length = 0x100
    
    u12_offset = \u4E00
    u12_length = 0x1000    

    U8  = [  u8_offset, ...,  u8_offset  + u8_length [
    U12 = [ u12_offset, ..., u12_offset + u12_length [

`U8` is actually the "Cyrillic" block, while `U12` is a subset of the "CJK Unified Ideographs" block.

Characters in `U8` can encode any 8-bits value by adding that value to the range starting offset. It goes the same with `U12`.

    0xAB  (8 bits)  gets encoded as \u0400 + 0xAB  = \u04AB = ҫ
    0xABC (12 bits) gets encoded as \u4E00 + 0xABC = \u58BC = 뱘

#### 2. Mapping Bytes to Unichars

UniBinary reads three bytes to yield two Unicode characters in the `U12` range .

Here is how UniBinary encode the 24 bits value `0xABCDEF` into two Unicode characters, and how Base64 does it by comparision:

            UniBinary                |                 Base64
                                     |
    A   B   |C   D   |E   F          |        A   B   |C   D   |E   F   
    10101011 11001101 11101111       |        10101011 11001101 11101111
    [-----------][-----------]       |        [----][-----][-----][----]
         ABC          DEF            |        101010 111100 110111 101111
    \u4E00+0xABC \u4E00+0xDEF        |          42     60     55     47
          墼           寯             |          q      8      3      v

At the end of the file, if less than three bytes are available, UniBinary reads bytes one by one to yield Unicode characters in `U8`.

    A   B   
    10101011
    [------]
       AB
    \u0400+0xAB
       ҫ

UniBinary also takes advantage of repetitions to spare bytes. A byte `B` repeated more that 3 times gets encoded as `u8 u12` where `u8` stores `B` and `u12` stores the number of times that `B` is repeated.

#### 3. Format Summary
    
    - u8  u12    ->    bytes B (u8) repeated N times (u12) | N in [3, 0xFFF]
    - u12 u12    ->    24 bits (3 bytes)
    - u8         ->    8 bits (1 byte)

UniBinary encoded data can be described with the following regular expression:

    ( (u12 u12) | (u8 u12) )* u8 {0,2}

#### 4. Examples

    0x12 0x34           -> encode 0x12 into u8, encode 0x34 into u8
    0xAB 0xCD 0xEF      -> encode 0xABC into u12, encode 0xDEF into u12
    0xFF 0xFF 0xFF 0xFF -> encode 0xFF into u8, encode 0x4 into u12

    AB CD EF FF FF FF FF 00 -> U12(0xABC), U12(0xDEF), U8(0xFF), U12(4), U8(00) -> "墼寯ᇿ丄ᄀ"

    13808 bytes /usr/bin/true -> 1913 Unicode characters, 7654 bytes UTF-16 files including BOM

#### 5. Encoded Text Size

UniBinary takes advantage of repetition to compress the data.

The worst case of encoding `N` bytes is `(N * 2 / 3 + 2)` Unicode characters.

`C` Unicode characters can store at least `(C - C % 3) * 3 / 2 + (C % 3)` bytes.

Hence, UniBinary can pack at least 209 bytes in the 140 characters of a Twitter message.

### Encoding Algorithm

First look for repetitions (but no more that 0xFFF). If no repeat, then try to consume three bytes. If less than three bytes available, encode one byte at a time.

    1. byte B repeated N times | N > 3    ->    U8(B), U12(N)
    2. bytes B1, B2, B3                   ->    U12(B1 << 4 + B2 >> 4), U12(((B2 & 0xF) << 8) + B3)
    3. byte B                             ->    U8(B)

The encoding algorithm is as follows:

    - if first byte available repeats more than R = 3 times:
       - write (u8, u12) where:
         u8 = u8_start_offset + the byte to repeat
         u12 = u12_start_offset + max(R, u12_length - 1)
    - else if at least 3 bytes available:
       - read three bytes b1, b2, b3
       - write (u12_1, u12_2) where:
         u12_1 = u12_start_offset + (b1 << 4) + (b2 >> 4)
         u12_2 = u12_start_offset + ((b2 & 0x0F) << 8) + b3
    - else if at least 1 byte available:
       - read one byte b1
       - write (u8) where:
         u8 = u8_start_offset + b1

### Decoding Algorithm

Read 2 Unicode chars if you can. It will be either `U12, U12` encoding 3 bytes, `U8, U8` encoding 2 bytes or `U8, U12` encoding the repeat of a byte. If only 1 unichar remains, it is in U8.

    - if at least 2 unichars available:
        - read two unichars u1 and u2
        - if u1 and u2 are both in U12
            - write (b1, b2, b3) where;
              i1 = u1 - u12_start_offset
              i2 = u2 - u12_start_offset
              b1 = i1 >> 4
              b2 = ((i1 & 0xF) << 4) + ((i2 & 0xF00) >> 8)
              b3 = i2 & 0x0FF
        - else if u1 and u2 are both in U8
            - write (b1, b2) where:
              b1 = u1 - u8_start_offset
              b2 = u2 - u8_start_offset
        - else if u1 in U8 and u2 in U12:
            - write byte b * n times, where
              b = u1 - u8_start_offset
              n = u2 - u12_start_offset
        - else:
            format error
    - else if at least 1 unichar available:
        - read 1 unichar u1
        - write b, where:
          b = u1 - u8_start_offset
