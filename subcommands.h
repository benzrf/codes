SUB(compress,   lzw_encode)
SUB(decompress, lzw_decode)
SUB(lzw_id,     lzw_encode, lzw_decode)
SUB(augment,    hamming_encode)
SUB(correct,    hamming_decode)
SUB(hamming_id, hamming_encode, hamming_decode)
SUB(encode,     lzw_encode, hamming_encode)
SUB(decode,     hamming_decode, lzw_decode)
SUB(full_id,    lzw_encode, hamming_encode, hamming_decode, lzw_decode)
#undef SUB

