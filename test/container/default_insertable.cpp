#include <stl2/detail/concepts/insertable.hpp>

namespace ranges = __stl2;
using ranges::models::DefaultInsertable;

static_assert(DefaultInsertable<bool>);
static_assert(DefaultInsertable<int>);
static_assert(DefaultInsertable<double>);

int main()
{}