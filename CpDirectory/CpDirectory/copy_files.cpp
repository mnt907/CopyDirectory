#include <iostream>
#include <string>

#ifdef _WIN32
#include <filesystem>
#include <Windows.h>
#include <fstream>
#else
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#endif //linux

namespace
{
#ifdef _WIN32
    namespace fs = std::experimental::filesystem;
#endif //WIN32

    std::string InputPath()
    {
        std::cout << "Enter the PATH" << std::endl;
        std::string user_input;
        std::cin >> user_input;
        std::cout << user_input << std::endl;
        return user_input;
    }

    bool IsExistFiles(const std::string& dir_path)
    {
        bool is_exist_file = false;

#ifdef _WIN32
        auto dir = fs::recursive_directory_iterator(fs::path(dir_path));

        for (auto& p : dir)
        {
            is_exist_file = true;
            break;
        }
#else
        DIR *dir_dirp = NULL;
        if ((dir_dirp = opendir(dir_path.c_str())) == NULL)
        {
            std::cout << "can't open directory" << dir_path << std::endl;
            return false;
        }
        const struct dirent *dp;
        if ((dp = readdir(dir_dirp)) == NULL)
        {
            std::cout << "can't read directory" << dir_path << std::endl;
            return false;
        }

        while ((dp = readdir(dir_dirp)) != NULL)
        {
            is_exist_file = true;
            break;
        }

        closedir(dir_dirp);

#endif //_WIN32

        return is_exist_file;
    }

    bool IsExistDiretory(const std::string& dir_path)
    {
#ifdef _WIN32
        if (fs::is_directory(fs::path(dir_path)))
#else
        DIR *src_dirp = NULL;
        if ((src_dirp = opendir(dir_path.c_str())) != NULL)
#endif
        {
            std::cout << "exists Directory" << std::endl;
            return true;
        }

        std::cout << "not exists Directory" << std::endl;
#ifndef _WIN32
        closedir(src_dirp);
#endif
        return false;
    }

    bool CopyFiles(const std::string& src_path, const std::string& dst_path)
    {
        FILE* src_fp = NULL;
        if ((src_fp = fopen(src_path.c_str(), "rb")) == NULL)
        {
            std::cout << "can't create read_filepointer" << std::endl;
            return false;
        }

        FILE* dst_fp = NULL;
        if ((dst_fp = fopen(dst_path.c_str(), "wb")) == NULL)
        {
            std::cout << "can't create write_filepointer" << std::endl;
            return false;
        }

        fseek(src_fp, 0, SEEK_END);
        int file_size = ftell(src_fp);
        rewind(src_fp);

        int file_buf_size = 256;

        char* file_buf = (char*)malloc(file_buf_size);
        memset(file_buf, 0, file_buf_size);

        int copy_number = file_size / file_buf_size;
        int last_file_size = file_size % file_buf_size;

        for (int i = 0; i < copy_number; i++)
        {
            memset(file_buf, 0, file_buf_size);
            fread(file_buf, file_buf_size, 1, src_fp);
            fwrite(file_buf, file_buf_size, 1, dst_fp);
            memset(file_buf, 0, file_buf_size);
        }

        if (last_file_size > 0)
        {
            free(file_buf);
            file_buf = (char*)malloc(last_file_size);
            memset(file_buf, 0, last_file_size);
            fread(file_buf, last_file_size, 1, src_fp);
            fwrite(file_buf, last_file_size, 1, dst_fp);
            memset(file_buf, 0, last_file_size);
        }

        free(file_buf);
        fclose(src_fp);
        fclose(dst_fp);

        return true;
    }

    bool CopyDirectory(const std::string& src_path, const std::string& dst_path)
    {
#ifdef _WIN32
        using namespace std;
        auto dir = fs::recursive_directory_iterator(fs::path(src_path));

        int src_path_length = src_path.length();
        cout << src_path_length << endl;

        for (const auto& file : dir)
        {
            string file_path = file.path().generic_string();
            file_path.assign(file_path.c_str(), src_path.length(), file_path.length());
            
            const string dst_file_path = dst_path + file_path;
            const string src_file_path = src_path + file_path;

            if (IsExistDiretory(src_file_path))
            {
                fs::create_directory(dst_file_path);
            }
            else
            {
                CopyFiles(src_file_path, dst_file_path);
            }
        }
        return true;
#else
        DIR *src_dirp = NULL;
        if ((src_dirp = opendir(src_path.c_str())) == NULL)
        {
            std::cout << "can't open directory" << src_path << std::endl;
            return false;
        }

        DIR *dst_dirp = NULL;
        if ((dst_dirp = opendir(dst_path.c_str())) == NULL)
        {
            std::cout << "can't open directory " << dst_path << std::endl;
            return false;
        }

        const struct dirent *dp;
        if ((dp = readdir(src_dirp)) == NULL)
        {
            std::cout << "can't read directory" << src_path << std::endl;
            return false;
        }

        struct stat file_info;
        while ((dp = readdir(src_dirp)) != NULL)
        {
            const std::string filename = dp->d_name;
            const std::string new_src_path = src_path + "/" + filename;
            const std::string new_dst_path = dst_path + "/" + filename;

            if ((lstat(new_src_path.c_str(), &file_info)) == -1)
            {
                std::cout << "can't read file info " << new_src_path << std::endl;
                continue;
            }

            if (S_ISDIR(file_info.st_mode))
            {
                if (filename == "." || filename == "..")
                    continue;
                else
                {
                    std::cout << "[directory name] " << new_src_path << std::endl;

                    mkdir(new_dst_path.c_str(), file_info.st_mode);
                    CopyDirectory(new_src_path, new_dst_path);
                }
            }
            else
            {
                std::cout << "[file name] " << new_src_path << std::endl;
                CopyFiles(new_src_path, new_dst_path);
                chmod(new_dst_path.c_str(), file_info.st_mode);
                chown(new_dst_path.c_str(), file_info.st_uid, file_info.st_gid);
            }
        }

        closedir(src_dirp);
        closedir(dst_dirp);

        return true;
#endif //_WIN32
    }
}

int main()
{
#ifdef _WIN32
    const std::string src_path = "C:/Users/mnt/Desktop/empty_directory";
    const std::string dst_path = "C:/Users/mnt/Desktop/dd";
#else
    const std::string src_path("/home/mnt907/study/src");
    const std::string dst_path("/home/mnt907/study/dst");
#endif

    for (int i = 0; i < 10000; ++i)
    {
        if (IsExistDiretory(src_path) == false)
            return 0;

        if (IsExistDiretory(dst_path) == false)
            return 0;

        if (IsExistFiles(src_path) == false)
            return 0;

        if (CopyDirectory(src_path, dst_path) == false)
            return 0;

        Sleep(200);
    }

    return 0;

}