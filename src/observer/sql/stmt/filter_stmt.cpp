/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#include "sql/stmt/filter_stmt.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "common/rc.h"
#include "common/type/attr_type.h"
#include "common/value.h"
#include "sql/expr/expression.h"
#include "sql/parser/parse_defs.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include <memory>
#include <regex>

RC resolve_unbound_field_expr(std::unique_ptr<Expression> &expr, Table *default_table, std::unordered_map<std::string, Table *> *tables);

FilterStmt::~FilterStmt()
{
  for (FilterUnit *unit : filter_units_) {
    delete unit;
  }
  filter_units_.clear();
}

RC FilterStmt::create(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
    const ConditionSqlNode *conditions, int condition_num, FilterStmt *&stmt)
{
  RC rc = RC::SUCCESS;
  stmt  = nullptr;

  FilterStmt *tmp_stmt = new FilterStmt();
  for (int i = 0; i < condition_num; i++) {
    FilterUnit *filter_unit = nullptr;

    rc = create_filter_unit(db, default_table, tables, conditions[i], filter_unit);
    if (rc != RC::SUCCESS) {
      delete tmp_stmt;
      LOG_WARN("failed to create filter unit. condition index=%d", i);
      return rc;
    }
    tmp_stmt->filter_units_.push_back(filter_unit);
  }

  stmt = tmp_stmt;
  return rc;
}

// RC get_table_and_field(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
//     const RelAttrSqlNode &attr, Table *&table, const FieldMeta *&field)
// {
//   if (common::is_blank(attr.relation_name.c_str())) {
//     table = default_table;
//   } else if (nullptr != tables) {
//     auto iter = tables->find(attr.relation_name);
//     if (iter != tables->end()) {
//       table = iter->second;
//     }
//   } else {
//     table = db->find_table(attr.relation_name.c_str());
//   }
//   if (nullptr == table) {
//     LOG_WARN("No such table: attr.relation_name: %s", attr.relation_name.c_str());
//     return RC::SCHEMA_TABLE_NOT_EXIST;
//   }

//   field = table->table_meta().field(attr.attribute_name.c_str());
//   if (nullptr == field) {
//     LOG_WARN("no such field in table: table %s, field %s", table->name(), attr.attribute_name.c_str());
//     table = nullptr;
//     return RC::SCHEMA_FIELD_NOT_EXIST;
//   }

//   return RC::SUCCESS;
// }

RC FilterStmt::create_filter_unit(Db *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
    const ConditionSqlNode &condition, FilterUnit *&filter_unit)
{
  RC rc = RC::SUCCESS;

  CompOp comp = condition.comp;
  if (comp < EQUAL_TO || comp >= NO_OP) {
    LOG_WARN("invalid compare operator : %d", comp);
    return RC::INVALID_ARGUMENT;
  }

  filter_unit = new FilterUnit;
  ConditionSqlNode &condition_ref = const_cast<ConditionSqlNode &>(condition);  // 转为引用类型，方便修改
  std::unique_ptr<Expression> left_expr(nullptr);
  std::unique_ptr<Expression> right_expr(nullptr);
  left_expr.reset(condition_ref.left_expr);
  right_expr.reset(condition_ref.right_expr);

  rc = resolve_unbound_field_expr(left_expr, default_table, tables);
  if (rc != RC::SUCCESS) {
    delete filter_unit;
    return rc;
  }

  rc = resolve_unbound_field_expr(right_expr, default_table, tables);
  if (rc != RC::SUCCESS) {
    delete filter_unit;
    return rc;
  }

  filter_unit->set_left(std::move(left_expr));
  filter_unit->set_right(std::move(right_expr));

  filter_unit->set_comp(comp);

  // 检查两个类型是否能够比较
  return rc;
}

// 递归函数：解析并替换表达式中的 UnboundFieldExpr 为 FieldExpr
RC resolve_unbound_field_expr(std::unique_ptr<Expression> &expr, Table *default_table, std::unordered_map<std::string, Table *> *tables) {
  if (!expr) {
    return RC::SUCCESS;
  }

  if (expr->type() == ExprType::VALUE) { 
    if (expr->value_type() == AttrType::CHARS) {
      ValueExpr *value_expr = static_cast<ValueExpr *>(expr.get());

      Value value;
      value_expr->get_value(value);
      std::string str_value = value.get_string();
      std::regex pattern(R"(^\d{4}-\d{1,2}-\d{1,2}$)");
      if (std::regex_match(str_value, pattern)) {
        Value date_value;
        RC rc = Value::cast_to(value, AttrType::DATES, date_value);
        if (rc != RC::SUCCESS) {
          LOG_WARN("Failed to cast date value: %s", str_value.c_str());
          return rc;
        }
        expr.reset(new ValueExpr(date_value));
        return RC::SUCCESS;
      }
    }
  }

  // 如果当前表达式是 UnboundFieldExpr，则进行解析并替换为 FieldExpr
  if (expr->type() == ExprType::UNBOUND_FIELD) {
    UnboundFieldExpr *unbound_field_expr = static_cast<UnboundFieldExpr *>(expr.get());
    const std::string &table_name = unbound_field_expr->table_name();
    const std::string &field_name = unbound_field_expr->field_name();
    Table *table = default_table;

    // 如果有表名，找到对应的表；否则使用默认表
    if (!table_name.empty()) {
      auto it = tables->find(table_name);
      if (it != tables->end()) {
        table = it->second;
      } else {
        LOG_WARN("Table '%s' not found in available tables.", table_name.c_str());
        return RC::SCHEMA_TABLE_NOT_EXIST;
      }
    }

    const FieldMeta *field_meta = table->table_meta().field(field_name.c_str());
    if (!field_meta) {
      LOG_WARN("Field '%s' not found in table '%s'.", field_name.c_str(), table->name());
      return RC::SCHEMA_FIELD_NOT_EXIST;
    }

    // 将 UnboundFieldExpr 替换为 FieldExpr
    expr.reset(new FieldExpr(table, field_meta));
    return RC::SUCCESS;
  }

  // 如果是 ArithmeticExpr，递归处理左右子表达式
  if (expr->type() == ExprType::ARITHMETIC) {
    ArithmeticExpr *arithmetic_expr = static_cast<ArithmeticExpr *>(expr.get());
    RC rc = resolve_unbound_field_expr(arithmetic_expr->left(), default_table, tables);
    if (rc != RC::SUCCESS) {
      return rc;
    }
    rc = resolve_unbound_field_expr(arithmetic_expr->right(), default_table, tables);
    if (rc != RC::SUCCESS) {
      return rc;
    }
  }

  // 其他类型的表达式可以直接返回
  return RC::SUCCESS;
}