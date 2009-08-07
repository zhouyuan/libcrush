#include <string.h>
#include "rgw_access.h"
#include "rgw_fs.h"
#include "rgw_rados.h"

static S3FS fs_provider;
static S3Rados rados_provider;

S3Access* S3Access::store;

S3Access *S3Access::init_storage_provider(const char *type, int argc, char *argv[])
{
  if (strcmp(type, "rados") == 0) {
    store = &rados_provider;
  } else if (strcmp(type, "fs") == 0) {
    store = &fs_provider;
  } else {
    store = NULL;
  }

  if (store->initialize(argc, argv) < 0)
    store = NULL;

  return store;
}