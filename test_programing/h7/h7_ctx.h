#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>

namespace h7 {
    using String = std::string;
    using CString = const std::string&;
template<typename k, typename v>
    using HashMap = std::unordered_map<k,v>;
template<typename T>
    using List = std::vector<T>;
template<typename T>
    using CList = const std::vector<T>&;
    using ListI = List<int>;
    using ListS = List<String>;
}
