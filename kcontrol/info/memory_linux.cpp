#include <syscall.h>
#include <linux/kernel.h>
#include <unistd.h>

void KMemoryWidget::update()
{
  struct sysinfo info;

  syscall(SYS_sysinfo, &info);	// get Information from system...

  Memory_Info[TOTAL_MEM]    = MEMORY(info.totalram);  // total physical memory (without swaps)
  Memory_Info[FREE_MEM]     = MEMORY(info.freeram);   // total free physical memory (without swaps)
  Memory_Info[SHARED_MEM]   = MEMORY(info.sharedram); 
  Memory_Info[BUFFER_MEM]   = MEMORY(info.bufferram); 
  Memory_Info[SWAP_MEM]     = MEMORY(info.totalswap); // total size of all swap-partitions
  Memory_Info[FREESWAP_MEM] = MEMORY(info.freeswap);  // free memory in swap-partitions
}
