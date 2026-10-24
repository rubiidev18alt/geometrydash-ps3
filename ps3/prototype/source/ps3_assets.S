    .section .rodata
    .balign 16
    .global gdps3_asset_blob_start
    .global gdps3_asset_blob_end
gdps3_asset_blob_start:
    .incbin "../data/ps3_assets.bin"
gdps3_asset_blob_end:

#ifdef __linux__
    .section .note.GNU-stack,"",@progbits
#endif
