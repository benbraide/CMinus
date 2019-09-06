#pragma once

namespace cminus::evaluator{
	enum class id{
		nil,
		undefined,
		initializer,
		boolean,
		byte,
		character,
		floating_point,
		integral,
		nan,
		pointer,
		class_,
	};
}
