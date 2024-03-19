#include "tlm.h"
#include "top.h"

int sc_main(int, char **argv) {
  const char *renode_address = argv[1];
  const char *renode_port = argv[2];

  top top("top", renode_address, renode_port);

  sc_core::sc_start();

  return 0;
}
