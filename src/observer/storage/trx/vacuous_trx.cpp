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
#include "common/type/attr_type.h"

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

RC VacuousTrx::update_record(Table *table, const std::string &field_name, const Value &new_value, Record &record) {
    RC update_result = RC::SUCCESS;

    // 验证输入参数
    if (nullptr == table || nullptr == record.data()) {
        LOG_ERROR("输入参数无效: table 或 record data 为空");
        return RC::INVALID_ARGUMENT;
    }

    // 获取字段元数据
    const FieldMeta *field_meta = table->table_meta().field(field_name.c_str());
    if (nullptr == field_meta) {
        LOG_ERROR("未找到字段: %s", field_name.c_str());
        return RC::SCHEMA_FIELD_NOT_EXIST;
    }

    // 验证字段长度与新值的长度
    if (new_value.length() > field_meta->len()) {
        LOG_ERROR("新值长度 (%d) 超过字段长度限制 (%d)", new_value.length(), field_meta->len());
        return RC::INVALID_ARGUMENT;
    }

    // 为新记录分配内存
    size_t record_size = table->table_meta().record_size();
    char *new_data = static_cast<char *>(malloc(record_size));
    if (nullptr == new_data) {
        LOG_ERROR("为新记录数据分配内存失败");
        return RC::NOMEM;
    }

    // 复制原始记录数据
    memcpy(new_data, record.data(), record_size);

    // 根据字段类型处理更新
    switch (field_meta->type()) {
        case AttrType::CHARS: { // 定长字符串类型
            // 先清空字段区域
            memset(new_data + field_meta->offset(), 0, field_meta->len());
            // 复制新值
            memcpy(new_data + field_meta->offset(), new_value.data(), new_value.length());
            // 用空格填充剩余部分
            if (new_value.length() < field_meta->len()) {
                memset(new_data + field_meta->offset() + new_value.length(), 
                       ' ', // 空格字符
                       field_meta->len() - new_value.length());
            }
            break;
        }
        case AttrType::INTS: 
        case AttrType::FLOATS: {   // 定长数值类型
            // 直接覆盖，确保完整的类型长度都被更新
            memcpy(new_data + field_meta->offset(), new_value.data(), field_meta->len());
            break;
        }
        default: {
            LOG_ERROR("不支持的字段类型: %d", field_meta->type());
            free(new_data);
            return RC::INTERNAL;
        }
    }

    // 更新记录
    update_result = table->update_record(record.rid(), new_data);
    if (update_result != RC::SUCCESS) {
        LOG_ERROR("更新记录失败. rc=%s", strrc(update_result));
    }

    // 清理内存
    free(new_data);

    return update_result;
}

RC VacuousTrx::visit_record(Table *table, Record &record, ReadWriteMode) { return RC::SUCCESS; }

RC VacuousTrx::start_if_need() { return RC::SUCCESS; }

RC VacuousTrx::commit() { return RC::SUCCESS; }

RC VacuousTrx::rollback() { return RC::SUCCESS; }

RC VacuousTrx::redo(Db *, const LogEntry &) { return RC::SUCCESS; }
