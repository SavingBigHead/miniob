#pragma once

#include "sql/operator/physical_operator.h"

class Trx;
class UpdateStmt;

/**
 * @brief 更新物理操作符
 */
class UpdatePhysicalOperator : public PhysicalOperator
{
public:
  UpdatePhysicalOperator(Table *table, const std::string &field_name, const Value &new_value)
      : table_(table), field_name_(field_name), new_value_(new_value) {}

  ~UpdatePhysicalOperator() = default;

  PhysicalOperatorType type() const override { return PhysicalOperatorType::UPDATE; }

  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;

private:
  Table              *table_ = nullptr;            // 需要更新的表
  std::string         field_name_;                 // 要更新的字段名
  Value               new_value_;                  // 新的字段值
  Trx                *trx_ = nullptr;              // 当前事务
  std::vector<Record> records_;                    // 匹配的记录列表
};
