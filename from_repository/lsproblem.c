#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>

void write_files(const char* namedir){

    int n;
    int i = 0;
    int j = 0;
    int jj = 0;
    struct dirent *namelist = NULL;
    struct dirent current;
    struct stat buf;
    struct group *gr;
    struct passwd *pwd;
    char* string;
    char* filename = (char*)malloc(1000);
    uid_t id;
    DIR* dir = NULL;
    dir = opendir(namedir);

    if (dir == NULL){perror("opendir");exit;}
    else{
        printf("%s \n", namedir);
        while(1)
        {
            namelist = NULL;
            n = readdir_r(dir, &current, &namelist);
            if(namelist == NULL){break;}
            else{
                if((strcmp(current.d_name,".")!=0)&&(strcmp(current.d_name,"..")!=0)){
                strcpy(filename, namedir);
                strcat(filename, "/");
                strcat(filename, current.d_name);
                  // if (stat(current.d_name, &buf) == -1) {
                if (stat(filename, &buf) == -1) {
                    //  printf("%s \n", current.d_name);
                      perror("stat");
                      exit;
                   }
                   else
                   {
                        if(current.d_type == DT_DIR){printf("d");}
                        else{
                            if(current.d_type == DT_LNK){ printf("l");}
                            else{printf("-");}
                        }

	                if(S_IRUSR & buf.st_mode){ printf("r");}
                        else{printf("-");}
                        if(S_IWUSR & buf.st_mode){printf("w");}
                        else{printf("-");}
                        if(S_IXUSR & buf.st_mode){printf("x");}
                        else{printf("-");}
                        if(S_IRGRP & buf.st_mode){printf("r");}
                        else{printf("-");}
                        if(S_IWGRP & buf.st_mode){printf("w");}
                        else{printf("-");}
                        if(S_IXGRP & buf.st_mode){printf("x");}
                        else{printf("-");}
                        if(S_IROTH & buf.st_mode){printf("r");}
                        else{printf("-");}
                        if(S_IWOTH & buf.st_mode){printf("w");}
                        else{printf("-");}
                        if(S_IXOTH & buf.st_mode){printf("x");}
                        else{printf("-");}
                        printf(" ");

                        switch (buf.st_mode & S_IFMT)
                        {
                            case S_IFBLK: printf("blk ");break;
                            case S_IFCHR: printf("chr ");break;
                            case S_IFDIR: printf("dir ");break;
                            case S_IFIFO: printf("fifo "); break;
                            case S_IFLNK: printf("lnk "); break;
                            case S_IFREG: printf("reg "); break;
                            case S_IFSOCK: printf("sock ");break;
                            default: printf("unknown "); break;
                        }


                        printf("%ld ", (long) buf.st_nlink);
                        gr = getgrgid(buf.st_gid);
                        if(gr!=NULL)printf("%-8.8s ", gr->gr_name);

                        pwd = getpwuid(buf.st_uid);
                        if (pwd != NULL)printf(" %-8.8s", pwd->pw_name);

                        printf("%lld ", (long long) buf.st_size);

                        string = ctime(&buf.st_mtime);

                        for(jj = 0; jj < strlen(string) - 1; jj++)
                            {
                                printf("%c", string[jj]);
                            }
                        printf(" ");
                        printf("%s ", current.d_name);
                        printf("\n");
                    }
                }

             }
        }
    n = closedir(dir);
    dir = opendir(namedir);
    while(1){
        namelist = NULL;
        n = readdir_r(dir, &current, &namelist);

        if(namelist == NULL) break;

        if (current.d_name[0] != '.'){
            strcpy(filename, namedir);
            strcat(filename, "/");
            strcat(filename, current.d_name);
            if (stat(filename, &buf) == -1) {
                perror("stat");
            }
            else{
                if(S_ISDIR(buf.st_mode)){
                   write_files(filename);
                }
            }
        }
    }
    closedir(dir);
    }
    free(filename);
}

int main(int argc, char* argv[])
{
    if(argc > 1)
    {
        write_files(argv[1]);
    }
    else{
        write_files(".");
    }
}

