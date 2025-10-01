import subprocess
import os
import sys

def get_executable_name(path: str) -> str:
    with open(path, "r") as file:
        executable = None

        for line in file:
            if "NAME" in line:
                return line.split('=')[1].strip()

        if not executable:
            raise Exception("""NAME variable Not defined\nMake sure the path is a Makefile and the NAME rule defined in the first line""")


def run_make(path):
    result = subprocess.run(["make"], capture_output=True, text=True, cwd=path)

    if result.returncode != 0:
        print(result.stderr)
        sys.exit(1)
    else:
        print(result.stdout)

def run_executable(EXECUTABLE, dir, config_path):
    exe_path = os.path.join(dir, EXECUTABLE)
    if not os.path.isfile(exe_path):
        print(f"❌ Executable '{exe_path}' not found. Did compilation succeed?")
        sys.exit(1)

    subprocess.run([f"./{EXECUTABLE}", config_path], cwd=dir)


if __name__ == "__main__":
    path = "/Users/oel-asri/Kingsave/Webserv/"
    exect = get_executable_name(path + "Makefile")
    run_make(path)
    run_executable(exect, path, "/Users/oel-asri/Kingsave/Webserv/response_unit_testing/file.config")
