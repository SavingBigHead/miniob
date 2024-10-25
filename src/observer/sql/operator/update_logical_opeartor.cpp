#include "sql/operator/update_logical_operator.h"

UpdateLogicalOperator::UpdateLogicalOperator(Table *table, const std::string &field_name, const Value &new_value, FilterStmt *filter_stmt = nullptr)
      : table_(table), field_name_(field_name), new_value_(new_value), filter_stmt_(filter_stmt) {}