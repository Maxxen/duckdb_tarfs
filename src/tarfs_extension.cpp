#include "tarfs_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension/extension_loader.hpp"

#include "tar_file_system.hpp"

namespace duckdb {

static void LoadInternal(ExtensionLoader &loader) {
	// Register the tar file system
	auto &db = loader.GetDatabaseInstance();
	auto &fs = db.GetFileSystem();
	fs.RegisterSubSystem(make_uniq<TarFileSystem>());
}

void TarfsExtension::Load(ExtensionLoader &loader) {
	LoadInternal(loader);
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

DUCKDB_CPP_EXTENSION_ENTRY(tarfs, loader) {
	duckdb::LoadInternal(loader);
}
}
