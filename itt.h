#ifndef ITT_INTERFACE_H
#define ITT_INTERFACE_H

#include <string>

#ifdef HAS_ITT

#include "ittnotify.h"

#else
typedef void __itt_domain;
typedef void __itt_string_handle;
typedef int __itt_id;

#define __itt_null NULL

__itt_domain *__itt_domain_create(const char *name) {
  return 0;
}
__itt_string_handle *__itt_string_handle_create(const char *name) {
  return 0;
}

void __itt_task_begin(const __itt_domain *domain,
                      __itt_id taskid,
                      __itt_id parentid,
                      __itt_string_handle *name) {}
void __itt_task_end(const __itt_domain *domain) {}

#endif

class BeginTask {
  __itt_domain* domain;
  bool inTask;

public:
  BeginTask(std::string domain_name, std::string name) : inTask(true) {
    domain = __itt_domain_create(domain_name.c_str());
    __itt_string_handle* handle_main = __itt_string_handle_create(name.c_str());
    __itt_task_begin(domain, __itt_null, __itt_null, handle_main);
  }

  void end() {
    if (inTask) {
      __itt_task_end(domain);
    }
  }

  ~BeginTask() {
    if (inTask) {
      __itt_task_end(domain);
    }
    domain = nullptr;
  }
};

#endif