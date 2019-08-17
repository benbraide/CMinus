#pragma once

#include "storage_object.h"

namespace cminus::logic{
	struct runtime{
		std::shared_ptr<storage::object> global_storage_;
		std::shared_ptr<storage::object> current_storage_;
		memory::object memory_object;
	};
}
