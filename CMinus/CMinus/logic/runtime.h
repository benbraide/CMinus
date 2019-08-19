#pragma once

#include "storage_object.h"

namespace cminus::logic{
	struct runtime{
		memory::object &memory_object;
		io::writer &writer;

		std::shared_ptr<storage::object> global_storage;
		std::shared_ptr<storage::object> current_storage;

		std::list<storage::object *> look_up_storages;
		std::shared_ptr<memory::reference> self;
	};

	class exception : public std::exception{
	public:
		using base_type = std::exception;

		exception(const char *message, std::size_t line, std::size_t column);

		std::size_t get_line() const;

		std::size_t get_column() const;

	private:
		std::size_t line_;
		std::size_t column_;
	};
}
