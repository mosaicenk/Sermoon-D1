import os, shutil, tempfile
Import("env")

# Relocate firmware from 0x08000000 to 0x08007000
env['CPPDEFINES'] = [d for d in env['CPPDEFINES'] if not (isinstance(d, tuple) and d[0] == "VECT_TAB_ADDR")]
env['CPPDEFINES'].append(("VECT_TAB_ADDR", "0x08007000"))

# Resolve linker script path — handle non-ASCII project directories.
# GCC ARM linker (arm-none-eabi-ld) cannot open files whose path contains
# non-ASCII characters (e.g. Turkish ı in "Drive'ım").
project_dir = env.get("PROJECT_DIR", "")
src_ld = os.path.join(project_dir, "buildroot", "share", "PlatformIO", "ldscripts", "creality.ld")

if os.name == 'nt':
    import ctypes
    from ctypes import wintypes
    _GetShortPathNameW = ctypes.windll.kernel32.GetShortPathNameW
    _GetShortPathNameW.argtypes = [wintypes.LPCWSTR, wintypes.LPWSTR, wintypes.DWORD]
    _GetShortPathNameW.restype = wintypes.DWORD
    buf = ctypes.create_unicode_buffer(259)
    short_ok = _GetShortPathNameW(src_ld, buf, 259) > 0
    if short_ok:
        custom_ld_script = buf.value
        try:
            custom_ld_script.encode('ascii')
        except UnicodeEncodeError:
            short_ok = False
    if not short_ok:
        # Short path still has non-ASCII — copy to ASCII-only temp location
        tmp_dir = os.path.join(tempfile.gettempdir(), "sermoon_d1_build")
        if not os.path.isdir(tmp_dir):
            os.makedirs(tmp_dir, exist_ok=True)
        tmp_ld = os.path.join(tmp_dir, "creality.ld")
        shutil.copy2(src_ld, tmp_ld)
        custom_ld_script = tmp_ld
else:
    custom_ld_script = os.path.normpath(src_ld)

for i, flag in enumerate(env["LINKFLAGS"]):
    if "-Wl,-T" in flag:
        env["LINKFLAGS"][i] = "-Wl,-T" + custom_ld_script
    elif flag == "-T":
        env["LINKFLAGS"][i + 1] = custom_ld_script
