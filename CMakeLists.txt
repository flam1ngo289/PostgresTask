cmake_minimum_required(VERSION 2.20)

project(PostgresTask)

set(PostgreSQL_ADDITIONAL_VERSIONS "16")

add_subdirectory("C:\\Users\\Кристина\\Desktop\\VS\\libpqxx-7.9.1" libpqxx-build)

add_executable(PostgresTask main.cpp)

target_compile_features(PostgresTask PRIVATE cxx_std_17)

target_link_libraries(PostgresTask PRIVATE pqxx)