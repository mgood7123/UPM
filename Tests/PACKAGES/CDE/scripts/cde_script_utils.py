import subprocess

def run_cmd(args):
  (cmd_stdout, cmd_stderr) = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
  return (cmd_stdout, cmd_stderr)


def is_dynamic_ELF_exe(filename):
  file_out, _ = run_cmd(['file', filename])
  return ("ELF" in file_out and "executable" in file_out and "dynamically linked" in file_out)
