#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>

char *extention[3]={".pdf", ".doc", ".txt"};
static const char *default_dir = "/home/falnerz/Documents";

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;
    char new_path[256];
    sprintf(new_path,"%s%s",default_dir,path);
	res = lstat(new_path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    DIR *dp;
	struct dirent *de;
    char new_path[256];
    if( !strcmp(path,"/") ){
        path=default_dir;
//        sprintf(new_path,"%s",);
        strcpy(new_path,default_dir);
    }
    else sprintf(new_path,"%s%s",default_dir,path);

	(void) offset;
	(void) fi;

	dp = opendir(new_path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}
/*


*/

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
 	int fd;
	int res;
	char new_path[256];
	(void) fi;

    sprintf(new_path,"%s%s",default_dir,path);

    fd = open(new_path, O_RDONLY);
	if (fd == -1){
		return -errno;
	}
	else{
        char ext[256];

        int u;
        for(u = 0; u < strlen(new_path) && new_path[u] != '.'; u++);
        strcpy(ext, new_path+u);
        for(int ext_idx=0;ext_idx<3;++ext_idx){
            if(!strcmp(ext,extention[ext_idx])){
                 system("zenity --error --title 'Error' --text 'Terjadi Kesalahan! File berisi konten berbahaya.'");
                    char rahasia[256];
                    strcpy(rahasia,new_path);
                    for(int i=strlen(rahasia)-1;rahasia[i]!='/';i--)rahasia[i]='\0';
                    strcat(rahasia,"rahasia");

                    struct stat s;
                    if (stat(rahasia, &s) != 0)mkdir(rahasia,0777);

                    strcat(rahasia,"/");
                    for(int i=strlen(new_path)-1; ;i--){
                        if(new_path[i]=='/'){
                            strcat(rahasia,new_path+(i+1));
                            break;
                        }
                    }
                    strcat(rahasia,".ditandai");
                    char command[256];
                    sprintf(command,"mv %s %s",new_path,rahasia);
                    system(command);
                    sprintf(command,"chmod 000 %s",rahasia);
                    system(command);

                    return -errno;

            }
        }
        res = pread(fd, buf, size, offset);
        if (res == -1)res = -errno;
        close(fd);
	}
	return res;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}

