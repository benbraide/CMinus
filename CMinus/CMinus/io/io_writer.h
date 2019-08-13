#pragma once

#include <any>

namespace cminus::io{
	class writer{
	public:
		virtual ~writer() = default;

		virtual std::size_t get_offset() const = 0;

		virtual std::size_t get_size() const = 0;

		virtual std::size_t set(std::byte value, std::size_t size) = 0;

		virtual std::size_t write(const std::byte *buffer, std::size_t size) = 0;

		virtual std::size_t write_scalar(const std::any &buffer) = 0;

		template <typename target_type>
		std::size_t write_scalar(const target_type &buffer){
			return write_scalar(std::any(&buffer));
		}

		template <typename target_type>
		std::size_t write_buffer(const target_type *buffer, std::size_t count){
			return write((const std::byte *)buffer, (sizeof(target_type) * count));
		}
	};
}
