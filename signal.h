#ifndef SIGNAL_H
#define SIGNAL_H

struct signal_t {
  uint32_t id;
  uint64_t timestamp;
  char data[8];
};

#endif /* SIGNAL_H */
