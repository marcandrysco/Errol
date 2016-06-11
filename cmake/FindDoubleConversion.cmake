# - Try to find DoubleConversion
# Once done this will define
#  DOUBLE_CONVERSION_FOUND - System has LibXml2
#  DOUBLE_CONVERSION_INCLUDE_DIRS - The LibXml2 include directories
#  DOUBLE_CONVERSION_LIBRARIES - The libraries needed to use LibXml2

find_path(DOUBLE_CONVERSION_INCLUDE_DIR double-conversion/double-conversion.h)
find_library(DOUBLE_CONVERSION_LIBRARY NAMES double-conversion)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DoubleConversion  DEFAULT_MSG
	DOUBLE_CONVERSION_LIBRARY DOUBLE_CONVERSION_INCLUDE_DIR)

mark_as_advanced(DOUBLE_CONVERSION_INCLUDE_DIR DOUBLE_CONVERSION_LIBRARY )

set(DOUBLE_CONVERSION_LIBRARIES ${DOUBLE_CONVERSION_LIBRARY} )
set(DOUBLE_CONVERSION_INCLUDE_DIRS ${DOUBLE_CONVERSION_INCLUDE_DIR} )
