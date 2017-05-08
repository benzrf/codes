#!/usr/bin/env python
"""
 biterror.py

 Insert bit flip errors with a given probability.

 The probability of an error (bit flip) per bit is

    p = error_per_bit_probability = 2 ** - bitflip_probability_log2

 If the file is N bits long, then the expected number of errors is N*p.

 The probability of no error is exactly (1-p)**N
 which if p is small is approximately (1 - N*p).
 (This is part of a Taylor Series or Binomial Expansion;
 the next term is (N*(N-1)/2)*p**2.)

 So for example if the file is 8 lines of 64 bytes,
 this is is 8 lines * 64 bytes/line * 8 bits/byte = 4096 bits
 or 2**3 * 2**6 * 2**3 = 2**12 bits.

 If p is 2**-9 , then the expected number of errors is
 (2**-9) * (2**12) = 2**3 = 8, i.e. about 1 bit error per line.

 This reads from stdin and writes to stdout and so can be
 used as part of a unix pipleline :

   $ cat test_in.txt | ./biterror.py > test_out.txt

 Tested with python3 (and all its bytes vs ascii vs unicode confusions).

 Jim Mahoney | cs.marlboro.edu | MIT License | April 2017
"""
import sys
import random

try:
    bitflip_probability_log2 = int(sys.argv[1])
except IndexError:
    bitflip_probability_log2 = 10   #  i.e. 2**-10 chance of a bit flip

def odds(poweroftwo):
    """ Return True with odds 2**-poweroftwo , else False
        For example, odds(2) will be True roughly 1/4 of the time,
        odds(3) will be True about 1/8 of the time, and so on.
    """
    #
    # To test this look at for example this count of the errors
    #   >> (a,b) = (3,256)
    # len(list(filter(lambda x:x, [odds(a) for i in range(b)])))
    # which should give about b/2**a = 32.
    #
    # The implementation here uses python's getrandbits(n).
    # For example, for n=2, that returns 11, 10, 01, or 00.
    # 00 = 0 will happen with probability 1/4 = 1/2**2
    #
    return random.getrandbits(poweroftwo) == 0

# I tweaked this a bit to stream instead of reading
# the entire input into memory. Also, using "bytearray"
# instead of list.

CHUNK_SIZE = 0xFF
def stdin_chunks():
    chunk = sys.stdin.buffer.read(CHUNK_SIZE)
    while chunk != b'':
        yield bytearray(chunk)
        chunk = sys.stdin.buffer.read(CHUNK_SIZE)

def corrupt(dat):
    for i, byte in enumerate(dat):
        for j in range(8):
            if odds(bitflip_probability_log2):
                bitmask = 1 << j
                byte ^= bitmask
        dat[i] = byte

for chunk in stdin_chunks():
    corrupt(chunk)
    sys.stdout.buffer.write(chunk)
