#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static const char *dirpath = "/home/lerufic/Downloads";

static int xmp_getattr(const char *path, struct stat *stbuf)
{
  int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		res = (filler(buf, de->d_name, &st, 0));
			if(res!=0) break;
	}

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi){
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;
  int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_open(const char *path, struct fuse_file_info *fi) {
        char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

	int res;
        res = open(fpath, fi->flags);
        if (res == -1)
                return -errno;
        fi->fh = res;
        return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	int fd;
	int res;
	char new_path[256];
	char backup[256];
	(void) fi;

    sprintf(new_path,"%s%s",dirpath,path);
    printf(backup,"%s%s",dirpath,path);


    char ext[256];

    int u;
    for(u = 0; u < strlen(new_path) && new_path[u] != '.'; u++);
    strcpy(ext, new_path+u);

    char simpanan[256];
    strcpy(simpanan,new_path);
    for(int i=strlen(simpanan)-1;simpanan[i]!='/';i--)simpanan[i]='\0';
    strcat(simpanan,"simpanan");

    struct stat s;
    if (stat(simpanan, &s) != 0)mkdir(simpanan, 0777);

    strcat(simpanan,"/");
    for(int i=strlen(new_path)-1; ;i--){
        if(new_path[i]=='/'){
            strcat(simpanan,new_path+(i+1));
            break;
        }
    }
    char command[256];
    sprintf(command,"cp %s %s",new_path,simpanan);
    system(command);

	if(fi == NULL)
            fd = open(simpanan, O_WRONLY);
    else
            fd = fi->fh;
    
    if (fd == -1)
            return -errno;
    res = pwrite(fd, buf, size, offset);
    if (res == -1)
            res = -errno;
    if(fi == NULL)
            close(fd);
    return res;
}

static int xmp_truncate(const char *path, off_t size, struct fuse_file_info *fi)
{
	char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);

        int res;
        if (fi != NULL)
                res = ftruncate(fi->fh, size);
        else
                res = truncate(fpath, size);
        if (res == -1)
                return -errno;
        return 0;
}

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);


        int res;
        res = open(fpath, fi->flags, mode);
        if (res == -1)
                return -errno;
        fi->fh = res;
        return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
	.open		= xmp_open,
	.write		= xmp_write,
	.truncate	= xmp_truncate
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
