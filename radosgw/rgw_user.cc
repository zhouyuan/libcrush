#include <errno.h>

#include <string>
#include <map>

#include "rgw_access.h"
#include "rgw_acl.h"

#include "include/types.h"
#include "rgw_user.h"

using namespace std;

static string ui_bucket = USER_INFO_BUCKET_NAME;
static string ui_email_bucket = USER_INFO_EMAIL_BUCKET_NAME;

int s3_get_user_info(string user_id, S3UserInfo& info)
{
  bufferlist bl;
  int ret;
  char *data;
  struct s3_err err;

  ret = s3store->get_obj(ui_bucket, user_id, &data, 0, -1, NULL, NULL, NULL, NULL, NULL, true, &err);
  if (ret < 0) {
    return ret;
  }
  bl.append(data, ret);
  bufferlist::iterator iter = bl.begin();
  info.decode(iter); 
  free(data);
  return 0;
}

void s3_get_anon_user(S3UserInfo& info)
{
  info.user_id = S3_USER_ANON_ID;
  info.display_name.clear();
  info.secret_key.clear();
}

int s3_store_user_info(S3UserInfo& info)
{
  bufferlist bl;
  info.encode(bl);
  const char *data = bl.c_str();
  string md5;
  int ret;
  map<nstring,bufferlist> attrs;

  ret = s3store->put_obj(info.user_id, ui_bucket, info.user_id, data, bl.length(), NULL, attrs);

  if (ret == -ENOENT) {
    ret = s3store->create_bucket(info.user_id, ui_bucket, attrs);
    if (ret >= 0)
      ret = s3store->put_obj(info.user_id, ui_bucket, info.user_id, data, bl.length(), NULL, attrs);
  }

  if (ret < 0)
    return ret;

  if (!info.user_email.size())
    return ret;

  S3UID ui;
  ui.user_id = info.user_id;
  bufferlist uid_bl;
  ui.encode(uid_bl);
  ret = s3store->put_obj(info.user_id, ui_email_bucket, info.user_email, uid_bl.c_str(), uid_bl.length(), NULL, attrs);
  if (ret == -ENOENT) {
    map<nstring, bufferlist> attrs;
    ret = s3store->create_bucket(info.user_id, ui_email_bucket, attrs);
    if (ret >= 0)
      ret = s3store->put_obj(info.user_id, ui_email_bucket, info.user_email, uid_bl.c_str(), uid_bl.length(), NULL, attrs);
  }

  return ret;
}

int s3_get_uid_by_email(string& email, string& user_id)
{
  bufferlist bl;
  int ret;
  char *data;
  struct s3_err err;
  S3UID uid;

  ret = s3store->get_obj(ui_email_bucket, email, &data, 0, -1, NULL, NULL, NULL, NULL, NULL, true, &err);
  if (ret < 0) {
    return ret;
  }
  bl.append(data, ret);
  bufferlist::iterator iter = bl.begin();
  uid.decode(iter); 
  user_id = uid.user_id;
  free(data);
  return 0;
}

int s3_get_user_buckets(string user_id, S3UserBuckets& buckets)
{
  bufferlist bl;
  int ret = s3store->get_attr(ui_bucket, user_id, S3_ATTR_BUCKETS, bl);
  switch (ret) {
  case 0:
    break;
  case -ENODATA:
    return 0;
  default:
    return ret;
  }

  bufferlist::iterator iter = bl.begin();
  buckets.decode(iter);

  return 0;
}

int s3_put_user_buckets(string user_id, S3UserBuckets& buckets)
{
  bufferlist bl;
  buckets.encode(bl);
  int ret = s3store->set_attr(ui_bucket, user_id, S3_ATTR_BUCKETS, bl);

  return ret;
}