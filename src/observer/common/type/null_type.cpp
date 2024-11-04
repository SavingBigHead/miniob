#include "null_type.h"
#include "common/rc.h"

RC NullType::to_string(const Value& src, std::string& result) const {
  RC rc = RC::SUCCESS;
  result = "null";
  return rc;
}