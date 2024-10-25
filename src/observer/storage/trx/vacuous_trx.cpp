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
// Created by Wangyunlai on 2023/4/24.
//

#include "storage/trx/vacuous_trx.h"

RC VacuousTrxKit::init() { return RC::SUCCESS; }

const vector<FieldMeta> *VacuousTrxKit::trx_fields() const { return nullptr; }

Trx *VacuousTrxKit::create_trx(LogHandler &) { return new VacuousTrx; }

Trx *VacuousTrxKit::create_trx(LogHandler &, int32_t /*trx_id*/) { return nullptr; }

void VacuousTrxKit::destroy_trx(Trx *trx) { delete trx; }

Trx *VacuousTrxKit::find_trx(int32_t /* trx_id */) { return nullptr; }

void VacuousTrxKit::all_trxes(vector<Trx *> &trxes) { return; }

LogReplayer *VacuousTrxKit::create_log_replayer(Db &, LogHandler &) { return new VacuousTrxLogReplayer; }

////////////////////////////////////////////////////////////////////////////////

RC VacuousTrx::insert_record(Table *table, Record &record) { return table->insert_record(record); }

RC VacuousTrx::delete_record(Table *table, Record &record) { return table->delete_record(record); }

RC VacuousTrx::update_record(Table *table, const std::string &field_name, const Value &new_value, Record &record){ 
  RC update_result = RC::SUCCESS;

  // 获取字段元信息
  const FieldMeta *field_meta = table->table_meta().field(field_name.c_str());
  if (nullptr == field_meta) {
    LOG_ERROR("Field not found: %s", field_name.c_str());
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  // 构造新的记录数据
  char *new_data = (char *)malloc(table->table_meta().record_size());
  memcpy(new_data, record.data(), table->table_meta().record_size());

  // 确保 new_value 的大小符合 field_meta->len()
  size_t value_len = field_meta->len();
  char *adjusted_value_data = (char *)malloc(value_len);
  memset(adjusted_value_data, 0, value_len);  // 初始化为0，避免垃圾值

  // 将 new_value 数据复制到调整后的缓冲区
  memcpy(adjusted_value_data, new_value.data(), std::min(sizeof(new_value.data()), value_len));

  // 将调整后的数据复制到新的记录数据中
  memcpy(new_data + field_meta->offset(), adjusted_value_data, value_len);

  // 调用 Table::update_record 更新记录
  update_result = table->update_record(record.rid(), new_data);
  if (update_result != RC::SUCCESS) {
    LOG_ERROR("Failed to update record. rc=%s", strrc(update_result));
  }

  // 释放内存
  free(new_data);
  free(adjusted_value_data);

  return update_result;
}


RC VacuousTrx::visit_record(Table *table, Record &record, ReadWriteMode) { return RC::SUCCESS; }

RC VacuousTrx::start_if_need() { return RC::SUCCESS; }

RC VacuousTrx::commit() { return RC::SUCCESS; }

RC VacuousTrx::rollback() { return RC::SUCCESS; }

RC VacuousTrx::redo(Db *, const LogEntry &) { return RC::SUCCESS; }
