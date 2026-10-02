#include <mulle-dtostr/mulle-dtostr.h>
#include <stdio.h>

int main(void) {
  char buf[MULLE__DTOSTR_BUFFER_SIZE];
  mulle_dtostr(6.62607015e-34, buf);
  puts(buf);
  return 0;
}
