find_path(LIBUNWIND_INCLUDE_DIR libunwind.h)
find_library(LIBUNWIND_LIBRARY unwind)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libunwind DEFAULT_MSG LIBUNWIND_LIBRARY LIBUNWIND_INCLUDE_DIR)
mark_as_advanced(LIBUNWIND_INCLUDE_DIR LIBUNWIND_LIBRARY)
