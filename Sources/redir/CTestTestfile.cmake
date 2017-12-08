# CMake generated Testfile for 
# Source directory: /home/universalpackagemanager/UPM/Sources/redir
# Build directory: /home/universalpackagemanager/UPM/Sources/redir
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test "test" "redir" "$<TARGET_FILE:mkdir_test>")
subdirs("tests/relocation/mkdir")
