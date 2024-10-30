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

#include "sql/stmt/update_stmt.h"
#include "common/log/log.h"
#include "storage/db/db.h"
#include "sql/stmt/filter_stmt.h"

UpdateStmt::UpdateStmt(Table *table, std::string field_name, Value values, FilterStmt *filter_stmt)
    : table_(table), field_name_(std::move(field_name)), values_(std::move(values)), filter_stmt_(filter_stmt)
{}

UpdateStmt::~UpdateStmt()
{
  if (filter_stmt_ != nullptr) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
}
RC UpdateStmt::create(Db *db, const UpdateSqlNode &update, Stmt *&stmt)
{
  // TODO
  stmt = nullptr;

  const char *table_name = update.relation_name.c_str();
  if (db == nullptr || table_name == nullptr || update.value.data() == nullptr) {
    LOG_WARN("invalid argument. db=%p, table_name=%p, value_num=%s",
        db, table_name, update.value.to_string().c_str());
    return RC::INVALID_ARGUMENT;
  }

  Table *table = db->find_table(table_name);
  if (table == nullptr) {
    LOG_WARN("table %s not found", table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  const FieldMeta *field_meta = table->table_meta().field(update.attribute_name.c_str());
  if (field_meta == nullptr) {
    LOG_WARN("field %s not found in table %s", update.attribute_name.c_str(), table_name);
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  if (field_meta->type() != update.value.attr_type()) {
    LOG_WARN("field %s type %d not match value type %d",
        update.attribute_name.c_str(), field_meta->type(), update.value.attr_type());
    return RC::SCHEMA_FIELD_TYPE_MISMATCH;
  }

  std::unordered_map<std::string, Table *> table_map;
  table_map.insert(std::make_pair(std::string(table_name), table));

  FilterStmt *filter_stmt = nullptr;
  RC          rc          = FilterStmt::create(
      db, table, &table_map, update.conditions.data(), static_cast<int>(update.conditions.size()), filter_stmt);

  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  const char *field_name = update.attribute_name.c_str();
  Value       value      = update.value;
  stmt                   = new UpdateStmt(table, std::string(field_name), std::move(value), filter_stmt);
  return RC::SUCCESS;
}
