#pragma once

#include "sql/operator/logical_operator.h"
#include "sql/stmt/filter_stmt.h"

/**
 * @brief 更新逻辑算子
 * @ingroup LogicalOperator
 */
class UpdateLogicalOperator : public LogicalOperator
{
public:
  /**
   * @brief 构造函数
   * @param table 待更新的表
   * @param field_name 要更新的字段名
   * @param new_value 新的字段值
   * @param filter_stmt 条件过滤器（可选）
   */
  UpdateLogicalOperator(Table *table, const std::string &field_name, const Value &new_value, FilterStmt *filter_stmt);

  virtual ~UpdateLogicalOperator() = default;

  LogicalOperatorType type() const override { return LogicalOperatorType::UPDATE; }

  Table                 *table() const { return table_; }
  const std::string     &field_name() const { return field_name_; }
  const Value           &new_value() const { return new_value_; }
  FilterStmt            *filter_stmt() const { return filter_stmt_; }

private:
  Table       *table_ = nullptr;            // 待更新的表
  std::string  field_name_;                 // 需要更新的字段名
  Value        new_value_;                  // 要更新的新值
  FilterStmt  *filter_stmt_ = nullptr;      // 条件过滤器，可能为空
};
