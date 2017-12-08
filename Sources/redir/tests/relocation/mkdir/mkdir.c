// test program for redir //
#include <sys/stat.h> // needed for mkdir()
#include <sys/types.h> // needed for mkdir()
#include <unistd.h> // needed for rmdir()
int main() {
    mkdir("tmp",777);
    // "/tmp" is already present
    mkdir("./tmp",777);
    mkdir("../tmp",777);
    mkdir("tmp/one",777);
    mkdir("/tmp/one",777);
    mkdir("./tmp/one",777);
    mkdir("../tmp/one",777);
    rmdir("tmp/one");
    rmdir("/tmp/one");
    rmdir("./tmp/one");
    rmdir("../tmp/one");
    rmdir("tmp");
    // "/tmp" should not be removed
    rmdir("./tmp");
    rmdir("../tmp");
}
