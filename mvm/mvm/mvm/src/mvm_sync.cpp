
#include "mvm_sync.h"
#include "mvm_type.h"

static uint32_t sync_count = 0;

void Sync_Start() {
    if(sync_count < 0xFFFFFFFF) {
        sync_count++;
        // TODO
        // Do something to start sync
    }
}

void Sync_Stop() {
    if(sync_count) {
        if(--sync_count == 0) {
            // TODO
            // Do something to stop sync
        }
    }
}
