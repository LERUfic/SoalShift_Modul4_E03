#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>

static const char *dirpath = "/home/praktikum/Downloads";

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

static int xmp_chmod(const char *path, mode_t mode)
{
 int res;
 char fpath[1000];
 sprintf(fpath,"%s%s",dirpath,path);
    res = chmod(fpath, mode);
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

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
	int res = 0;
	int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1){

        return -errno;
	}
	else{
	    res = pread(fd, buf, size, offset);
	    if (res == -1)
	        res = -errno;

	    close(fd);
	    return res;
	}
}

static int xmp_rename(const char *from, const char *to)
{
	int res;
	char new_from[1000];
	char new_to[1000];
	sprintf(new_from,"%s%s",dirpath,from);
	sprintf(new_to,"%s%s",dirpath,to);
    res = rename(new_from,new_to);
	if(res == -1)
	 return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s", dirpath, path);

	res = truncate(fpath, size);
	if(res == -1)
		return -errno;

	return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s", dirpath, path);
	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

    char command[256];
    sprintf(command,"mv '%s' %s.copy",fpath,fpath);
    system(command);
 	close(fd);
	return res;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
	char fpath[1000];
	sprintf(fpath,"%s%s", dirpath, path);
	res = mknod(fpath, mode, rdev);
	if(res == -1)
	    return -errno;

	return 0;
}



static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
    char ext[256];
    int u;
    for(u = strlen(fpath)-1; u>-1 && fpath[u] != '.'; u--);
    strcpy(ext,fpath+u);
    if( !strcmp(ext,".copy") ){
        system("zenity --error --title 'Error' --text 'File yang anda buka adalah file hasil salinan. File tidak bisa diubah maupun disalin kembali!'");
        return -errno;
    }

	int res;
	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}


static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)return -errno;
	return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr 	= xmp_getattr,
	.readdir 	= xmp_readdir,
	.read 		= xmp_read,
	.rename 	= xmp_rename,
	.write 		= xmp_write,
	.mknod 		= xmp_mknod,
	.truncate 	= xmp_truncate,
	.open 		= xmp_open,
	.chmod 		= xmp_chmod,
	.utimens 	= xmp_utimens,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}

