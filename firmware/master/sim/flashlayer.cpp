#include <stdlib.h>
#include "flashlayer.h"
#include <sifteo/macros.h>

FlashLayer::CachedBlock FlashLayer::blocks[NUM_BLOCKS];

char* FlashLayer::getRegion(uintptr_t address, int len)
{
    (void)len;
    return (char*)address;
#if 0 // eventually
    CachedBlock *b = getCachedBlock(address);
    if (b != 0) {
        if (address + len > b->address + sizeof(b->data)) {
            // requested region crosses block boundary :(
            return 0;
        }
        return b->data + (address - b->address);
    }
    // XXX - pull from external flash via DMA
    return 0;
#endif
}

char* FlashLayer::getRegionFromOffset(int offset, int len, int *size)
{
    if (mFile == NULL) {
        fprintf(stdout, "ERROR: asset segment file not open\n");
    }
    
    CachedBlock *b = getCachedBlock(offset);
    if (b == 0) {
        //fprintf(stdout, "Cache miss for offset: %d\n", offset);
        
        b = getFreeBlock();
        if (b == 0) {
            fprintf(stdout, "ERROR: No free blocks in cache\n");
            return 0;
        }
        
        int bpos = offset / BLOCK_SIZE;
        ASSERT(fseek(mFile, bpos * BLOCK_SIZE, SEEK_SET) == 0);
        
        size_t result = fread (b->data, 1, BLOCK_SIZE, mFile);
        if (result != (size_t)BLOCK_SIZE) {
            // TODO: Should we pad asset files to 512?  If not, zero out the remainder of this block.
            *size = result;
        }
        
        b->address = bpos * BLOCK_SIZE;
        b->inUse = true;
        b->valid = true;
    } else {
        //LOG(("Cache miss for offset: %d\n", offset));
        //fprintf(stdout, "Cache miss for offset: %d\n", offset);
    }
    
    int boff = offset % BLOCK_SIZE;
    *size = len;
    
    if ((unsigned)offset + len > b->address + sizeof(b->data)) {
        // requested region crosses block boundary :(
        int valid = b->address + BLOCK_SIZE - offset;
        *size = valid;
    }
    
    return b->data + boff;
}

void FlashLayer::releaseRegion(uintptr_t address)
{
    (void)address;
}

// File hacks

FILE * FlashLayer::mFile = NULL;

void FlashLayer::init()
{
    fprintf(stdout, "FlashLayer::init\n");
    
    mFile = fopen("asegment.bin", "rb");
    if (mFile == NULL) {
        fprintf(stdout, "ERROR: failed to open asset segment\n");
        return;
    }
}
