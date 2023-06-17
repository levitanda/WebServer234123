#ifndef __REQUEST_H__

void requestHandle(int fd, int *sthread, int *dthread, int *sumup_thread, int index, struct timeval received, struct timeval handeling);

#endif
