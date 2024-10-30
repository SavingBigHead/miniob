#include "vector_type.h"
#include "common/value.h"

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