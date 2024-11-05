    typedef struct ShadowRegionsDataStructure
    {
        uintptr_t OriginalPML4_t;
        uintptr_t ClonedPML4_t;
        uintptr_t GameCr3;
        uintptr_t ClonedCr3;
        uintptr_t FreeIndex;
    } ShadowRegionsDataStructure;
 
    // simplified decryption for non-HVCI mode; HVCI version varies a little, use the decryption routine from vgk.sys for it
    auto decrypt_cloned_cr3_simplified(uintptr_t cloned_cr3) -> uintptr_t
    {
        uintptr_t Last = cloned_cr3 & 0xFFFFFFFFF;
        uintptr_t FirstDigit = Last >> 32 & 0xF;
        uintptr_t LastDigit = Last & 0xF;
 
        FirstDigit -= 1;
        LastDigit -= 1;
 
        return FirstDigit << 32 | Last & 0x0FFFFFFF0 | LastDigit;
    }
 
    auto find_pml4_base() -> uintptr_t
    {
        auto vgk = utils::get_kernel_module("vgk.sys");
        if (!vgk) {
            dbg("vgk not found!");
            return 0;
        }
 
        auto ShadowRegionsData = *(ShadowRegionsDataStructure*)(vgk + 0x89488);
        if (!ShadowRegionsData.GameCr3) {
            dbg("ShadowRegionsData not found!");
            return 0;
        }
 
        // set read/write translation context to the decrypted cloned cr3
        context_cr3 = decrypt_cloned_cr3_simplified(ShadowRegionsData.ClonedCr3);
 
        return ShadowRegionsData.FreeIndex << 0x27;
    }
 
	auto uworld = driver.read< uintptr_t >( pml4base + offsets::uworld );
	printf( "uworld: 0x%p\n", uworld );