#pragma once

#include "common/type/attr_type.h"
#include "common/type/data_type.h"
#include "common/value.h"

class NullType : public DataType {
  public:
    NullType() : DataType(AttrType::NULLS) {}
    virtual ~NullType() {}

    RC to_string(const Value& value, std::string& str) const override;
};