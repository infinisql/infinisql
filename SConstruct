import os

def which(file):
    for path in os.environ["PATH"].split(":"):
        if os.path.exists(path + "/" + file):
                return path + "/" + file
    return None

env = DefaultEnvironment()
env.Replace(CXX=which('g++'))

SConscript('src/SConscript')

env.Clean('distclean', ['.sconsign.dblite', '.sconf_temp', 'config.log'])