#pragma once

#include <string>
#include <vector>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/mpl.hpp>

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3.hpp>

#define CMINUS_AST_JOIN_(x, y) x ## y
#define CMINUS_AST_JOIN(x, y) CMINUS_AST_JOIN_(x, y)

#define CMINUS_AST_NAME(n) CMINUS_AST_JOIN(n, _ast)
#define CMINUS_AST_QNAME(n) cminus::parsing::CMINUS_AST_NAME(n)

#define CMINUS_AST_FORWARD_NAME(n) boost::spirit::x3::forward_ast<CMINUS_AST_NAME(n)>
#define CMINUS_AST_FORWARD_QNAME(n) boost::spirit::x3::forward_ast<CMINUS_AST_QNAME(n)>

#define CMINUS_AST_SINGLE_FIELD_TYPE value_type
#define CMINUS_AST_SINGLE_FIELD_NAME value

#define CMINUS_AST_MULTIPLE_FIRST_FIELD_TYPE first_value_type
#define CMINUS_AST_MULTIPLE_FIRST_FIELD_NAME first_value

#define CMINUS_AST_MULTIPLE_SECOND_FIELD_TYPE second_value_type
#define CMINUS_AST_MULTIPLE_SECOND_FIELD_NAME second_value

#define CMINUS_AST_MULTIPLE_THIRD_FIELD_TYPE third_value_type
#define CMINUS_AST_MULTIPLE_THIRD_FIELD_NAME third_value

#define CMINUS_AST_TEMPLATE_TYPE(n, ...) n<__VA_ARGS__>
#define CMINUS_AST_VARIANT(...) CMINUS_AST_TEMPLATE_TYPE(boost::variant, __VA_ARGS__)

#define CMINUS_AST_WPOS boost::spirit::x3::position_tagged

#define CMINUS_AST_DECLARE_NAME(n) struct CMINUS_AST_NAME(n);

#define CMINUS_AST_DECLARE_SINGLE(n, t)																	\
struct CMINUS_AST_NAME(n){																				\
	using CMINUS_AST_SINGLE_FIELD_TYPE = t;																\
	CMINUS_AST_SINGLE_FIELD_TYPE CMINUS_AST_SINGLE_FIELD_NAME;											\
};

#define CMINUS_AST_DECLARE_SINGLE_WPOS(n, t)															\
struct CMINUS_AST_NAME(n) : CMINUS_AST_WPOS{															\
	using CMINUS_AST_SINGLE_FIELD_TYPE = t;																\
	CMINUS_AST_SINGLE_FIELD_TYPE CMINUS_AST_SINGLE_FIELD_NAME;											\
};

#define CMINUS_AST_DECLARE_SINGLE_VARIANT(n, ...)														\
struct CMINUS_AST_NAME(n){																				\
	using CMINUS_AST_SINGLE_FIELD_TYPE = CMINUS_AST_VARIANT(__VA_ARGS__);								\
	CMINUS_AST_SINGLE_FIELD_TYPE CMINUS_AST_SINGLE_FIELD_NAME;											\
};

#define CMINUS_AST_DECLARE_SINGLE_VARIANT_WPOS(n, ...)													\
struct CMINUS_AST_NAME(n) : CMINUS_AST_WPOS{															\
	using CMINUS_AST_SINGLE_FIELD_TYPE = CMINUS_AST_VARIANT(__VA_ARGS__);								\
	CMINUS_AST_SINGLE_FIELD_TYPE CMINUS_AST_SINGLE_FIELD_NAME;											\
};

#define CMINUS_AST_DECLARE_PAIR(n, t1, t2)																\
struct CMINUS_AST_NAME(n){																				\
	using CMINUS_AST_MULTIPLE_FIRST_FIELD_TYPE = t1;													\
	using CMINUS_AST_MULTIPLE_SECOND_FIELD_TYPE = t2;													\
	CMINUS_AST_MULTIPLE_FIRST_FIELD_TYPE CMINUS_AST_MULTIPLE_FIRST_FIELD_NAME;							\
	CMINUS_AST_MULTIPLE_SECOND_FIELD_TYPE CMINUS_AST_MULTIPLE_SECOND_FIELD_NAME;						\
};

#define CMINUS_AST_DECLARE_PAIR_WPOS(n, t1, t2)															\
struct CMINUS_AST_NAME(n) : CMINUS_AST_WPOS{															\
	using CMINUS_AST_MULTIPLE_FIRST_FIELD_TYPE = t1;													\
	using CMINUS_AST_MULTIPLE_SECOND_FIELD_TYPE = t2;													\
	CMINUS_AST_MULTIPLE_FIRST_FIELD_TYPE CMINUS_AST_MULTIPLE_FIRST_FIELD_NAME;							\
	CMINUS_AST_MULTIPLE_SECOND_FIELD_TYPE CMINUS_AST_MULTIPLE_SECOND_FIELD_NAME;						\
};

#define CMINUS_AST_DECLARE_TRIO(n, t1, t2, t3)															\
struct CMINUS_AST_NAME(n){																				\
	using CMINUS_AST_MULTIPLE_FIRST_FIELD_TYPE = t1;													\
	using CMINUS_AST_MULTIPLE_SECOND_FIELD_TYPE = t2;													\
	using CMINUS_AST_MULTIPLE_THIRD_FIELD_TYPE = t3;													\
	CMINUS_AST_MULTIPLE_FIRST_FIELD_TYPE CMINUS_AST_MULTIPLE_FIRST_FIELD_NAME;							\
	CMINUS_AST_MULTIPLE_SECOND_FIELD_TYPE CMINUS_AST_MULTIPLE_SECOND_FIELD_NAME;						\
	CMINUS_AST_MULTIPLE_THIRD_FIELD_TYPE CMINUS_AST_MULTIPLE_THIRD_FIELD_NAME;							\
};

#define CMINUS_AST_DECLARE_TRIO_WPOS(n, t1, t2, t3)														\
struct CMINUS_AST_NAME(n) : CMINUS_AST_WPOS{															\
	using CMINUS_AST_MULTIPLE_FIRST_FIELD_TYPE = t1;													\
	using CMINUS_AST_MULTIPLE_SECOND_FIELD_TYPE = t2;													\
	using CMINUS_AST_MULTIPLE_THIRD_FIELD_TYPE = t3;													\
	CMINUS_AST_MULTIPLE_FIRST_FIELD_TYPE CMINUS_AST_MULTIPLE_FIRST_FIELD_NAME;							\
	CMINUS_AST_MULTIPLE_SECOND_FIELD_TYPE CMINUS_AST_MULTIPLE_SECOND_FIELD_NAME;						\
	CMINUS_AST_MULTIPLE_THIRD_FIELD_TYPE CMINUS_AST_MULTIPLE_THIRD_FIELD_NAME;							\
};

#define CMINUS_AST_ADAPT_SINGLE(n)																		\
BOOST_FUSION_ADAPT_STRUCT(																				\
	CMINUS_AST_QNAME(n),																				\
	(CMINUS_AST_QNAME(n)::CMINUS_AST_SINGLE_FIELD_TYPE, CMINUS_AST_SINGLE_FIELD_NAME)					\
)

#define CMINUS_AST_ADAPT_PAIR(n)																		\
BOOST_FUSION_ADAPT_STRUCT(																				\
	CMINUS_AST_QNAME(n),																				\
	(CMINUS_AST_QNAME(n)::CMINUS_AST_MULTIPLE_FIRST_FIELD_TYPE, CMINUS_AST_MULTIPLE_FIRST_FIELD_NAME)	\
	(CMINUS_AST_QNAME(n)::CMINUS_AST_MULTIPLE_SECOND_FIELD_TYPE, CMINUS_AST_MULTIPLE_SECOND_FIELD_NAME)	\
)

#define CMINUS_AST_ADAPT_TRIO(n)																		\
BOOST_FUSION_ADAPT_STRUCT(																				\
	CMINUS_AST_QNAME(n),																				\
	(CMINUS_AST_QNAME(n)::CMINUS_AST_MULTIPLE_FIRST_FIELD_TYPE, CMINUS_AST_MULTIPLE_FIRST_FIELD_NAME)	\
	(CMINUS_AST_QNAME(n)::CMINUS_AST_MULTIPLE_SECOND_FIELD_TYPE, CMINUS_AST_MULTIPLE_SECOND_FIELD_NAME)	\
	(CMINUS_AST_QNAME(n)::CMINUS_AST_MULTIPLE_THIRD_FIELD_TYPE, CMINUS_AST_MULTIPLE_THIRD_FIELD_NAME)	\
)

namespace cminus::parsing{
	CMINUS_AST_DECLARE_SINGLE_WPOS(cminus_identifier, std::string)
}

CMINUS_AST_ADAPT_SINGLE(cminus_identifier)
