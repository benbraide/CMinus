#pragma once

#include <string>
#include <sstream>
#include <iomanip>

namespace cminus::logic{
	template <class value_type>
	struct to_string{
		static std::string get(value_type value){
			return std::to_string(value);
		}
	};

	template <>
	struct to_string<char>{
		static std::string get(char value){
			return std::string(1u, value);
		}
	};

	template <class value_type>
	struct floating_point_to_string{
		static std::string get(value_type value){
			auto str = std::to_string(value);
			if (str.empty())
				return str;

			if (auto pt_index = str.find('.'); pt_index < str.size()){
				auto last_index = (str.size() - 1u), last_zero_index = last_index;
				while ((pt_index + 1u) < last_zero_index && str[last_zero_index] == '0')
					--last_zero_index;

				if (last_zero_index < last_index)//Truncate zeros
					str.erase(last_zero_index, (last_index - last_zero_index));
			}
			else//Append point
				str += ".0";

			return str;
		}
	};

	template <>
	struct to_string<float>{
		static std::string get(float value){
			return floating_point_to_string<float>::get(value);
		}
	};

	template <>
	struct to_string<double>{
		static std::string get(double value){
			return floating_point_to_string<double>::get(value);
		}
	};

	template <>
	struct to_string<long double>{
		static std::string get(long double value){
			return floating_point_to_string<long double>::get(value);
		}
	};

	template <class value_type>
	struct to_hex_string{
		static std::string get(value_type value){
			std::stringstream stream;
			stream << "0x" << std::setfill('0') << std::setw(sizeof(value_type) * 2) << std::hex << value;
			return stream.str();
		}
	};
}
