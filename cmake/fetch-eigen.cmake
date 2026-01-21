message(STATUS "Third-party: load eigen...")

include(FetchContent)

set(EIGEN_VERSION "3.4.1")

# Configure Eigen options before fetching
set(EIGEN_BUILD_DOC OFF)
set(EIGEN_BUILD_TESTING OFF)
set(EIGEN_BUILD_PKGCONFIG OFF)
set(EIGEN_MPL2_ONLY ON) # Only code with Mozilla Public License 2.0 

FetchContent_Declare(
    eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/${EIGEN_VERSION}/eigen-${EIGEN_VERSION}.tar.gz
    URL_HASH SHA256=b93c667d1b69265cdb4d9f30ec21f8facbbe8b307cf34c0b9942834c6d4fdbe2
)

FetchContent_MakeAvailable(eigen)

# Eigen automatically provides Eigen3::Eigen, but for uniformity add custom alias
add_library(dependencies::eigen ALIAS eigen)

# Mark Eigen headers as SYSTEM to suppress warnings from third-party code
# Note: Eigen3::Eigen is an ALIAS, so we need to set properties on the actual target 'eigen'
get_target_property(EIGEN_INCLUDE_DIRS eigen INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(eigen PROPERTIES
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${EIGEN_INCLUDE_DIRS}"
)


