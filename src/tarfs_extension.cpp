#define DUCKDB_EXTENSION_MAIN

#include "tarfs_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"

#include "tar_file_system.hpp"

namespace duckdb {


static void LoadInternal(DatabaseInstance &instance) {
    // Register the tar file system
	auto &fs = instance.GetFileSystem();
	fs.RegisterSubSystem(make_uniq<TarFileSystem>());
}

void TarfsExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}
std::string TarfsExtension::Name() {
	return "tarfs";
}

std::string TarfsExtension::Version() const {
#ifdef EXT_VERSION_TARFS
	return EXT_VERSION_TARFS;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void tarfs_init(duckdb::DatabaseInstance &db) {
    duckdb::DuckDB db_wrapper(db);
    db_wrapper.LoadExtension<duckdb::TarfsExtension>();
}

DUCKDB_EXTENSION_API const char *tarfs_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
