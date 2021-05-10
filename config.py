from optparse import OptionParser
from pathlib import Path
import sys
import platform
import os
import re
import getpass

parser = OptionParser()
parser.add_option("--spath", type=str, default="src")
parser.add_option("--dpath", type=str, default="build")
parser.add_option("--version", type=str, default="1.0.0", help="ars version")
parser.add_option("--build-version", type=str, default="1.0.0", help="ars build version")
parser.add_option("--build-time", type=str, help="ars build time")
parser.add_option("--compiler", type=str, help="ars compiler")
parser.add_option("--debug", type=str, default="n", help="ars build time")
(options, args) = parser.parse_args()

def configure_file(input_file, output_file, vars_dict):
    
    with input_file.open('r') as f:
        template = f.read()

    for var in vars_dict:
        template = template.replace('@' + var + '@', vars_dict[var])

    with output_file.open('w') as f:
        f.write(template)

source_dir = Path(options.spath)
binary_dir = Path(options.dpath)
input_file = source_dir / 'configure.h.in'
output_file = binary_dir / 'configure.h'

sys.path.insert(0, str(source_dir))

versions = options.version.split(".")
versions = [int(item) for item in versions]

vars_dict = {
    'version':                  options.version,
    'version_major':            str(versions[0]),
    'version_minor':            str(versions[1]),
    'version_alter':            str(versions[2]),
    'version_build':            options.build_version,
    'build_time':               options.build_time,
    'mode':                     "debug" if options.debug == 'y' else "release",
    'debug':                    "0" if options.debug == 'n' else "1",
    'aru_athor':                'wotsen(astralrovers@outlook.com)',
    'plat':                     sys.platform,
    'arch':                     platform.machine(),
    'plat_version':             platform.version(),
    'processor':                platform.processor(),
    'os':                       platform.platform(),
    'release_user':             getpass.getuser(),
    'compiler':                 options.compiler,
}
configure_file(input_file, output_file, vars_dict)
