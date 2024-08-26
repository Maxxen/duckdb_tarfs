# TARFS DuckDB Extension

This repository is based on https://github.com/duckdb/extension-template, check it out if you want to build and ship your own DuckDB extension.

---

This extension, `tarfs`, provides a duckdb file-system abstraction to read and glob files within __uncompressed__ tar archives.

## Install from the community repository

This extension is (soon) available from the [DuckDB community extension repository](https://github.com/duckdb/community-extensions). To install the latest version simply run the following SQL statement from within a running DuckDB process.
```sql
    INSTALL tarfs FROM 'community';
```

If you already have an older version of the extension installed, you can upgrade to the latest version by running the following SQL statement.
```sql
    FORCE INSTALL tarfs FROM 'community';
```

For more information regarding community extensions, see the [blog post](https://duckdb.org/2024/07/05/community-extensions.html)

## Example usage

For example, given a tar file ab.tar containing two csv's, a.csv (1,2,3) and b.csv (4,5,6) you can do:

Glob into a tar archive:

```sql
SELECT filename FROM read_blob('tar://data/csv/tar/ab.tar/*') ORDER BY ALL;
----
tar://data/csv/tar/ab.tar/a.csv
tar://data/csv/tar/ab.tar/b.csv
```

Open a specific file inside of a tar archive:

```sql
SELECT * FROM read_csv('tar://data/csv/tar/ab.tar/a.csv') ORDER BY ALL;
----
1	2	3
```

Glob and open multiple files within the tar archive:

```sql
SELECT * FROM read_csv('tar://data/csv/tar/ab.tar/*') ORDER BY ALL;
----
1	2	3
4	5	6
```

`tarfs` can be used in combination with DuckDB's `httpfs` to read tar archives over http by chaining the `tar://` and `http://` prefixes. Heres an example:

```sql
SELECT filename FROM read_blob('tar://https://duckdb-blobs.s3.amazonaws.com/ab.tar/*') ORDER BY ALL;
----
tar://https://duckdb-blobs.s3.amazonaws.com/ab.tar/a.csv
tar://https://duckdb-blobs.s3.amazonaws.com/ab.tar/b.csv
```

For more examples, see the [tests](test/sql/tarfs.sql) included in this repository.

### Performance considerations

In order to avoid DuckDB repeatedly having to seek through the tar archive to find each individual file when globbing, you should make sure to enable DuckDB's "object cache" so that the byte offsets for each entry within the tar archive gets cached in memory. This can be done by setting the `SET enable_object_cache = true` setting.
Enabling the object cache can significantly speed up globbing and reading multiple files from a single tar archive when there are a large number of files in the archive, especially when reading over httpfs.

### Limitations

Because tar files don't contain something like an entry index or central directory structure to get the byte offsets of each entry, globbing and reading multiple files from a single archive has quadratic complexity as opening the nth file requires reading the headers of and seeking past all of the (n-1), (n-2), ..., (n-n-1) files preceding it. However, if the DuckDB object cache is enabled, the offsets of each file is cached when globbing, leading to only a single seek being required when later opening the file.

Because being able to seek within the tar archive is a requirement, it is currently NOT possible to open/glob and read files within a compressed tar archive, such as .tar.gz. We have some ideas on how to make this possible in the future but it would require significant changes to how DuckDB handles compressed files and probably require investing in a more sophisticated file caching mechanism.

However, you can still read compressed files within a tar archive, as long as the archive itself is not compressed.

Additionally, there are some limitations regarding globbing - recursive/nested globbing is not supported (unless the crawl is at the absolute end of the pattern) and it is not possible to glob within nested tar files (i.e. tar files within other tar files), although you can read files from them if you pass an absolute path.

## Development

### Building

`tarfs` requires no external dependencies, so using VCPKG is not necessary.

To build a release version of the extension, run:
```sh
make
```
The main binaries that will be built are:
```sh
./build/release/duckdb
./build/release/test/unittest
./build/release/extension/tarfs/tarfs.duckdb_extension
```
- `duckdb` is the binary for the duckdb shell with the extension code automatically loaded.
- `unittest` is the test runner of duckdb. Again, the extension is already linked into the binary.
- `tarfs.duckdb_extension` is the loadable binary as it would be distributed.

Alternatively you can build a debug version of the extension by running:
```sh
make debug
```

### Running the extension
To run the extension code, simply start the shell with `./build/release/duckdb`.

Now we can use the features from the extension directly in DuckDB. For example, to list the files in a tar archive, you can run:
```sql
SELECT filename FROM read_blob('tar://test/data/csv/tar/ab.tar/*') ORDER BY ALL;
```

See the [example usage](#example-usage) section for more examples.

### Running the tests
Different tests can be created for DuckDB extensions. The primary way of testing DuckDB extensions should be the SQL tests in `./test/sql`. These SQL tests can be run using:
```sh
make test
```

Alternatively, if you already have debug build compiled, you can simply run the test binary directly, providing the absolute path to the test you want to run. For unix-like systems you can use the following command to run them all:

```sh
./build/debug/test/unittest "$PWD/test/sql/*"
```

### Installing your own binaries
If you wish to host the extension binary yourself and make it installable from e.g. S3, you will need to do two things. Firstly, DuckDB should be launched with the
`allow_unsigned_extensions` option set to true. How to set this will depend on the client you're using. Some examples:

CLI:
```shell
duckdb -unsigned
```

Python:
```python
con = duckdb.connect(':memory:', config={'allow_unsigned_extensions' : 'true'})
```

NodeJS:
```js
db = new duckdb.Database(':memory:', {"allow_unsigned_extensions": "true"});
```

Secondly, you will need to set the repository endpoint in DuckDB to the HTTP url of your bucket + version of the extension
you want to install. To do this run the following SQL query in DuckDB:
```sql
SET custom_extension_repository='bucket.s3.eu-west-1.amazonaws.com/<your_extension_name>/latest';
```
Note that the `/latest` path will allow you to install the latest extension version available for your current version of
DuckDB. To specify a specific version, you can pass the version instead.

After running these steps, you can install and load your extension using the regular INSTALL/LOAD commands in DuckDB:
```sql
INSTALL tarfs
LOAD tarfs
```
