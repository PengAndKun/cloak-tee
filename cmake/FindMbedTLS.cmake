# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.

find_path(MBEDTLS_INCLUDE_DIRS mbedtls/ssl.h)

find_library(MBEDTLS_LIBRARY mbedtls)
find_library(MBEDCRYPTO_LIBRARY mbedcrypto)

set(MBEDTLS_LIBRARIES "${MBEDTLS_LIBRARY}" "${MBEDCRYPTO_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  MbedTLS DEFAULT_MSG MBEDTLS_INCLUDE_DIRS MBEDTLS_LIBRARY MBEDCRYPTO_LIBRARY
)

mark_as_advanced(
  MBEDTLS_INCLUDE_DIRS MBEDTLS_LIBRARY MBEDCRYPTO_LIBRARY
)
