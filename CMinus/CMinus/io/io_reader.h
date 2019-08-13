#pragma once

#include <any>

namespace cminus::io{
	class reader{
	public:
		virtual ~reader() = default;

		virtual std::size_t get_offset() const = 0;

		virtual std::size_t get_size() const = 0;

		virtual std::size_t get_bytes_remaining() const = 0;

		virtual std::size_t ignore(std::size_t size) const = 0;

		virtual std::size_t read(std::byte *buffer, std::size_t size) const = 0;

		virtual std::size_t read_scalar(const std::any &buffer) const = 0;

		template <typename target_type>
		target_type read_scalar() const{
			auto buffer = target_type();
			return ((read_scalar(std::any(&buffer)) == sizeof(target_type)) ? buffer : target_type());
		}

		template <typename target_type>
		std::size_t read_buffer(target_type *buffer, std::size_t count) const{
			return read((std::byte *)buffer, (sizeof(target_type) * count));
		}
	};
}
