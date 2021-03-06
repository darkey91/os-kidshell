#include <unistd.h>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sys/wait.h>

static const size_t ARGS_MAX = 32, FILE_NAME_MAX = 256;

void substr(const std::string &str, char *arg, size_t from, size_t to) {
    size_t index = 0;
    for (size_t i = from; i < to; ++i) {
        arg[index++] = str[i];
    }
    arg[index] = '\0';
}

size_t skip_spaces(const std::string &str, size_t index) {
    while (isspace(str[index])) ++index;
    return  index;
}

void get_first_arg(const std::string & path, char *arg_first) {
  size_t index = path.size() - 1;
  for (; index != 0 && path[index] != '/'; --index) {}
  substr(path, arg_first, !index ? 0 : index + 1, path.size());
}

void parse_args(const std::string & args_string, char *args[]) {
    size_t from = skip_spaces(args_string, 0), args_num = 0;
    for (size_t i = from; i < args_string.size(); ++i) {
        if (i == args_string.size() - 1) {
           substr(args_string, args[++args_num], from, args_string.size());
        } else if (isspace(args_string[i])) {
            substr(args_string, args[+args_num], from, i - from);
            i = skip_spaces(args_string, i);
            from = i;
        }
    }
    args[++args_num] = nullptr;
}

void print_current_path() {
    char cur_dir[FILE_NAME_MAX];
    getcwd(cur_dir, FILE_NAME_MAX);
    std::cout << cur_dir <<":~$ ";
}

void init_args(const std::string &path, const std::string &args_string, char *args[]) {
    get_first_arg(path, args[0]);
    parse_args(args_string, args);
}

int main() {
  std::string path, args_string;

  char **args = new char *[ARGS_MAX];
  char *envp[] = {nullptr};

  for (size_t i = 0; i < ARGS_MAX; ++i) {
      args[i] = new char[FILE_NAME_MAX];
  }

    while (true) {
        print_current_path();

        if (!(std::cin >> path))
            break;
        if (path.empty()) continue;
        getline(std::cin, args_string);

        init_args(path, args_string, args);

        pid_t pid = fork();

        if (pid == 0) {
            int program_status = execve(path.data(), args, envp);
            std::cerr << "Can't execute " << path << ": " <<  strerror(errno) << "\n";
            return program_status;
        } else if (pid == -1) {
            std::cerr << "Can't fork: " <<  strerror(errno) << "\n";
        } else {
            int wstatus;
            if (waitpid(pid, &wstatus, WUNTRACED) == -1) {
                std::cerr << "Error occured in child process: " << strerror(errno) << "\n";
            } else {
                if (WIFEXITED(wstatus)) {
                    int exit_status = WEXITSTATUS(wstatus);
                    std::cout << path << " execution exited with code " << exit_status << "\n";
                } else {
                    std::cerr << path << " execution didn't finish correctly.\n";
                }
            };
        }
    }

    for (size_t i = 0; i < ARGS_MAX; ++i)   {
        if (args[i] != nullptr)
            delete [] args[i];
    }
    delete [] args;

  return 0;
}
