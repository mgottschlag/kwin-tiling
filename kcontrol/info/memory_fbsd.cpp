// $Id$

#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdlib.h>	// For atoi()
#include <unistd.h>

void KMemoryWidget::update()
{
  char blah[10], buf[80], *used_str, *total_str;
  /* Stuff for sysctl */
  int mib[2],memory;size_t len;
  /* Stuff for swap display */
  int used, total, _free;
  FILE *pipe;
  
  mib[0]=CTL_HW;mib[1]=HW_PHYSMEM;
  len=sizeof(memory);
  sysctl(mib, 2, &memory, &len, NULL, 0);
  
  snprintf(blah, 10, "%d", memory);
  // Numerical values
  totalMem->setText(format(memory));
  /*	To: questions@freebsd.org
		Anyone have any ideas on how to calculate this */
  sharedMem->setText(i18n("Not available"));

  int buffers;
  len = sizeof (buffers);
  if ((sysctlbyname("vfs.bufspace", &buffers, &len, NULL, 0) == -1) || !len)
    bufferMem->setText(i18n("Not available")); // Doesn't work under FreeBSD v2.2.x
  else
    bufferMem->setText(format(buffers));

  int free;
  len = sizeof (buffers);
  if ((sysctlbyname("vm.stats.vm.v_free_count", &free, &len, NULL, 0) == -1) || !len)
    freeMem->setText(i18n("Not available")); // Doesn't work under FreeBSD v2.2.x
  else
    freeMem->setText(format(free*getpagesize()));

  /* Q&D hack for swap display. Borrowed from xsysinfo-1.4 */
  if ((pipe = popen("/usr/sbin/pstat -ks", "r")) == NULL) {
     used = total = 1;
     return;
  }

  fgets(buf, sizeof(buf), pipe);
  fgets(buf, sizeof(buf), pipe);
  fgets(buf, sizeof(buf), pipe);
  fgets(buf, sizeof(buf), pipe);
  pclose(pipe);

  strtok(buf, " ");
  total_str = strtok(NULL, " ");
  used_str = strtok(NULL, " ");
  used = atoi(used_str);
  total = atoi(total_str); 

  _free=total-used;
  swapMem->setText(format(1024*total));
  freeSwapMem->setText(format(1024*_free));
}
