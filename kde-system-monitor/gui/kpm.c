/* A small wrapper to call kde-system-monitor --showprocesses */

#include <unistd.h>

int main()
{
  return execlp( "kde-system-monitor", "kde-system-monitor", "--showprocesses", 0 );
}
