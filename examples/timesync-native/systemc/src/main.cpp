#include <systemc>
#include <tlm>
#include "renode_bridge_native.h"
#include "timekeeper.h"

top top("top", new timekeeper("m_timekeeper"));

int sc_main(int, char **argv) {
  return 0;
}
