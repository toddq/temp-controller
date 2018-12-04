#ifndef MY_SOFTAP_PAGES
#define MY_SOFTAP_PAGES

#include "Particle.h"
#include "softap_http.h"

// from https://docs.particle.io/reference/device-os/firmware/photon/#complete-example

void handleCustomPage(const char* url, ResponseCallback* cb, void* cbArg, Reader* body, Writer* result, void* reserved);

STARTUP(softap_set_application_page_handler(handleCustomPage, nullptr));

#endif
