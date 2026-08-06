#
# version-stamp.py
# Force a full rebuild when Marlin/Version.h changes.
#
# Version.h is included through a macro-expanded path
# (MarlinConfigPre.h:42 -> #include XSTR(../../CUSTOM_VERSION_FILE)), so the
# SCons C scanner cannot resolve the include path and Version.h never enters
# the dependency graph: an incremental build silently keeps the OLD version
# string in the binary.
#
# Injecting the file's content hash as a deliberately unused -D flag changes
# every compile command line whenever the version changes, which makes SCons
# rebuild everything -- the documented `rm -rf .pio/build/creality` step,
# automated.
#
import hashlib
import os

Import("env")

version_header = os.path.join(env.subst("$PROJECT_DIR"), "Marlin", "Version.h")

try:
    with open(version_header, "rb") as f:
        digest = hashlib.md5(f.read()).hexdigest()[:8]
except OSError:
    digest = None

if digest:
    env.Append(CCFLAGS=["-DVERSION_STAMP=0x" + digest])
else:
    print("version-stamp: WARNING: %s not found" % version_header)
