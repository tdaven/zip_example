#include <boost/filesystem.hpp>

#include <archive.h>
#include <archive_entry.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void add_file(archive* a, boost::filesystem::path const& parentPath, boost::filesystem::path const& childPath)
{
	fprintf(stderr, "add %s / %s\n", parentPath.c_str(), childPath.c_str());
	//
	auto entry = archive_entry_new();
	
	//
	auto fullPath = parentPath / childPath;
	
	//
	struct stat st = {};
	stat(fullPath.c_str(), &st);

	//
	archive_entry_set_pathname(entry, childPath.c_str());
	archive_entry_set_filetype(entry, AE_IFDIR);
	archive_entry_copy_stat(entry, &st);
	archive_write_header(a, entry);

	//
	auto fd = open(fullPath.c_str(), O_RDONLY);
	if(fd >= 0)
	{
		char buff[8192];     
		ssize_t bytes_read;
		while ((bytes_read = read(fd, buff, sizeof(buff))) > 0) 
		{
			archive_write_data(a, buff, bytes_read);
		}
		close(fd);
	}
	
	//
	archive_entry_free(entry);
	archive_write_finish_entry(a);
}

void compress(boost::filesystem::path const& outputPath, boost::filesystem::path const& inputPath)
{
	using namespace boost::filesystem;
    boost::filesystem::path parentPath(inputPath.parent_path());

	auto a = archive_write_new();
	archive_write_set_format_zip(a);
	archive_write_set_options(a, "zip:compression=deflate");
	archive_write_open_filename(a, outputPath.c_str());
	
	if(boost::filesystem::is_regular_file(inputPath))
	{
		auto childPath = boost::filesystem::relative(inputPath, parentPath);
		add_file(a, parentPath, childPath);
	}
	else
	{	
		boost::filesystem::recursive_directory_iterator dir(inputPath);
		boost::filesystem::recursive_directory_iterator end;
		while (dir != end)
		{
			auto childPath = boost::filesystem::relative(dir->path(), parentPath);
			add_file(a, parentPath, childPath);
			dir++;
		}
	}
	archive_write_close(a);
	archive_write_free(a);
}

int main(int argc, char* argv[])
{
		compress("/path/output.zip", "/path/compress");
    
	
	return 0;
}

