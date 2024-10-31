#include "vector_type.h"
#include "common/value.h"
#include <cmath>

int VectorType::compare(const Value &left, const Value &right) const
{
  std::vector<float> left_vec  = left.get_vector();
  std::vector<float> right_vec = right.get_vector();

  for (int i = 0; i < left_vec.size(); i++) {
    if (left_vec[i] < right_vec[i]) {
      return -1;
    } else if (left_vec[i] > right_vec[i]) {
      return 1;
    }
  }
  return 0;
}

RC VectorType::add(const Value &left, const Value &right, Value &result) const
{
  std::vector<float> left_vec  = left.get_vector();
  std::vector<float> right_vec = right.get_vector();
  std::vector<float> result_vec;
  for (int i = 0; i < left_vec.size(); i++) {
    result_vec.push_back(left_vec[i] + right_vec[i]);
  }
  result.set_vector(result_vec);
  return RC::SUCCESS;
}
RC VectorType::subtract(const Value &left, const Value &right, Value &result) const
{
  std::vector<float> left_vec  = left.get_vector();
  std::vector<float> right_vec = right.get_vector();
  std::vector<float> result_vec;
  for (int i = 0; i < left_vec.size(); i++) {
    result_vec.push_back(left_vec[i] - right_vec[i]);
  }
  result.set_vector(result_vec);
  return RC::SUCCESS;
}
RC VectorType::multiply(const Value &left, const Value &right, Value &result) const
{
  std::vector<float> left_vec  = left.get_vector();
  std::vector<float> right_vec = right.get_vector();
  std::vector<float> result_vec;
  for (int i = 0; i < left_vec.size(); i++) {
    result_vec.push_back(left_vec[i] * right_vec[i]);
  }
  result.set_vector(result_vec);
  return RC::SUCCESS;
}

RC VectorType::to_string(const Value &val, string &result) const
{
  std::vector<float> vec = val.get_vector();
  std::ostringstream oss;
  oss << "[";
  for (int i = 0; i < vec.size(); i++) {
    oss << vec[i];
    if (i < vec.size() - 1) {
      oss << ", ";
    }
  }
  oss << "]";
  result = oss.str();
  return RC::SUCCESS;
}

// L2 距离计算
RC VectorType::l2_distance(const Value &left, const Value &right, Value &result) const {
    std::vector<float> left_vec = left.get_vector();
    std::vector<float> right_vec = right.get_vector();
    if (left_vec.size() != right_vec.size()) {
        return RC::INVALID_ARGUMENT;
    }
    float sum = 0.0;
    for (int i = 0; i < left_vec.size(); i++) {
        float diff = left_vec[i] - right_vec[i];
        sum += diff * diff;
    }
    float distance = std::sqrt(sum);
    result.set_float(roundf(distance * 100) / 100); // 保留两位小数
    return RC::SUCCESS;
}

// 余弦距离计算
RC VectorType::cosine_distance(const Value &left, const Value &right, Value &result) const {
    std::vector<float> left_vec = left.get_vector();
    std::vector<float> right_vec = right.get_vector();
    if (left_vec.size() != right_vec.size()) {
        return RC::INVALID_ARGUMENT;
    }
    float dot_product = 0.0;
    float norm_left = 0.0;
    float norm_right = 0.0;
    for (int i = 0; i < left_vec.size(); i++) {
        dot_product += left_vec[i] * right_vec[i];
        norm_left += left_vec[i] * left_vec[i];
        norm_right += right_vec[i] * right_vec[i];
    }
    float cosine_similarity = dot_product / (std::sqrt(norm_left) * std::sqrt(norm_right));
    float distance = 1 - cosine_similarity;
    result.set_float(roundf(distance * 100) / 100); // 保留两位小数
    return RC::SUCCESS;
}

// 内积计算
RC VectorType::inner_product(const Value &left, const Value &right, Value &result) const {
    std::vector<float> left_vec = left.get_vector();
    std::vector<float> right_vec = right.get_vector();
    if (left_vec.size() != right_vec.size()) {
        return RC::INVALID_ARGUMENT;
    }
    float product = 0.0;
    for (int i = 0; i < left_vec.size(); i++) {
        product += left_vec[i] * right_vec[i];
    }
    result.set_float(roundf(product * 100) / 100); // 保留两位小数
    return RC::SUCCESS;
}