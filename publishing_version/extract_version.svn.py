import subprocess
import os
import re

def svnversion():
    p = subprocess.Popen("svnversion", stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (stdout, stderr) = p.communicate()
    return stdout

# get svn version
svn_version = bytes.decode(svnversion(), 'utf-8').strip()

# write to file
base_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '.'))
template_file = os.path.join(base_path, 'version.template')
header_file = os.path.join(base_path, 'version.h')

r_test_line = 'aaabbbccc $PATH_VERSION'
test_line = re.sub('\$PATH_VERSION', svn_version, r_test_line)

with open(template_file, 'r') as rfile:
    rline = rfile.read()
    wline = re.sub('\$PATH_VERSION', svn_version, rline)

with open(header_file, 'w') as wfile:
    wfile.write(wline)
